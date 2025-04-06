#include "overlay_value_display/overlay_value_display.hpp"
#include "overlay_value_display/msg/overlay_value_in_range.hpp" // Corrected include path

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <rviz_common/display_context.hpp>
#include <rviz_common/logging.hpp>

#include <sstream>
#include <iomanip>

namespace overlay_value_display
{

OverlayValueDisplay::OverlayValueDisplay() : rviz_common::Display()
{
  // Property initialization will happen in onInitialize
}

OverlayValueDisplay::~OverlayValueDisplay()
{
  if (overlay_) {
    overlay_->hide();
    Ogre::OverlayManager::getSingleton().destroy(overlay_);
    overlay_ = nullptr;
  }
}

void OverlayValueDisplay::onInitialize()
{
  rviz_common::Display::onInitialize();

  // Create properties
  topic_property_ = new rviz_common::properties::StringProperty(
    "Topic",
    "",
    "ROS topic to subscribe to for value updates.",
    getPropertyContainer(),
    SLOT(updateTopic()));

  text_color_property_ = new rviz_common::properties::ColorProperty(
    "Text Color",
    QColor(255, 255, 255),
    "Color of the min and max value text.",
    getPropertyContainer(),
    SLOT(updateOverlay()));

  font_size_property_ = new rviz_common::properties::IntProperty(
    "Font Size",
    12,
    "Font size of the min and max value text.",
    getPropertyContainer(),
    SLOT(updateOverlay()));

  graph_width_property_ = new rviz_common::properties::IntProperty(
    "Graph Width",
    500,
    "Width of the rectangular graph in pixels.",
    getPropertyContainer(),
    SLOT(updateOverlay()));

  graph_height_property_ = new rviz_common::properties::IntProperty(
    "Graph Height",
    50,
    "Height of the rectangular graph in pixels.",
    getPropertyContainer(),
    SLOT(updateOverlay()));

  display_orientation_property_ = new rviz_common::properties::EnumProperty(
    "Orientation",
    "Horizontal",
    "Orientation of the graph.",
    getPropertyContainer(),
    SLOT(updateOverlay()));
  display_orientation_property_->addOption("Horizontal", 0);
  display_orientation_property_->addOption("Vertical", 1);

  overlay_x_property_ = new rviz_common::properties::IntProperty(
    "Overlay X",
    10,
    "X position of the overlay in pixels.",
    getPropertyContainer(),
    SLOT(updateOverlay()));

  overlay_y_property_ = new rviz_common::properties::IntProperty(
    "Overlay Y",
    10,
    "Y position of the overlay in pixels.",
    getPropertyContainer(),
    SLOT(updateOverlay()));

  current_value_display_property_ = new rviz_common::properties::EnumProperty(
    "Current Value Display",
    "Line",
    "How to display the current value.",
    getPropertyContainer(),
    SLOT(updateOverlay()));
  current_value_display_property_->addOption("Line", 0);
  current_value_display_property_->addOption("Box", 1);

  // Create overlay
  static int count = 0;
  Ogre::OverlayManager* overlay_manager = Ogre::OverlayManager::getSingletonPtr();
  overlay_ = overlay_manager->create("OverlayValueDisplayOverlay" + std::to_string(count++));
  overlay_->setZOrder(500); // Ensure it's on top
  overlay_->show();

  // Create initial overlay elements (will be positioned and sized in updateOverlay)
  min_text_ = createOrUpdateText("MinText", "", 0.0f, 0.0f, 1.0f, 1.0f);
  max_text_ = createOrUpdateText("MaxText", "", 0.0f, 0.0f, 1.0f, 1.0f);
  graph_rect_ = createOrUpdateRect("GraphRect", 0.0f, 0.0f, 1.0f, 1.0f, Ogre::ColourValue(0.5f, 0.5f, 0.5f, 0.5f));
  current_line_ = createOrUpdateRect("CurrentLine", 0.0f, 0.0f, 0.002f, 1.0f, Ogre::ColourValue::Green); // Initial width
  current_box_ = createOrUpdateRect("CurrentBox", 0.0f, 0.0f, 0.1f, 0.8f, Ogre::ColourValue::Blue);     // Initial size
  current_value_text_ = createOrUpdateText("CurrentValueText", "", 0.0f, 0.0f, 1.0f, 1.0f);

  updateTopic();
  updateOverlay();
}

void OverlayValueDisplay::onEnable()
{
  if (value_sub_) {
    value_sub_->activate();
  }
  if (overlay_) {
    overlay_->show();
  }
}

void OverlayValueDisplay::onDisable()
{
  if (value_sub_) {
    value_sub_->deactivate();
  }
  if (overlay_) {
    overlay_->hide();
  }
}

void OverlayValueDisplay::updateTopic()
{
  if (isEnabled()) {
    value_sub_.reset();
    std::string topic_name = topic_property_->getStdString();
    if (!topic_name.empty()) {
      try {
        rclcpp::QoS qos(1); // Keep last message
        value_sub_ = this->getDisplayContext()->getRosNodeAbstraction()->get_node()->create_subscription<overlay_value_display::msg::OverlayValueInRange>(
          topic_name, qos, std::bind(&OverlayValueDisplay::valueCallback, this, std::placeholders::_1));
        RVIZ_COMMON_LOG_INFO("Subscribing to topic: " << topic_name);
      } catch (const rclcpp::exceptions::InvalidTopicNameError& e) {
        RVIZ_COMMON_LOG_ERROR("Error subscribing to topic '" << topic_name << "': " << e.what());
      }
    }
  }
}

void OverlayValueDisplay::valueCallback(const overlay_value_display::msg::OverlayValueInRange::SharedPtr msg)
{
  current_msg_ = msg;
  updateOverlay();
}

void OverlayValueDisplay::updateOverlay()
{
  if (!overlay_ || !current_msg_) {
    return;
  }

  Ogre::ColourValue textColor = text_color_property_->getOgre();
  int fontSize = font_size_property_->getInt();
  int graphWidth = graph_width_property_->getInt();
  int graphHeight = graph_height_property_->getInt();
  int overlayX = overlay_x_property_->getInt();
  int overlayY = overlay_y_property_->getInt();
  int orientation = display_orientation_property_->getOptionIndex();
  int currentValueDisplay = current_value_display_property_->getOptionIndex();

  // Convert pixel coordinates to viewport coordinates (0.0 to 1.0)
  float vp_x = static_cast<float>(overlayX) / getViewportWidth();
  float vp_y = static_cast<float>(overlayY) / getViewportHeight();
  float vp_graph_width = static_cast<float>(graphWidth) / getViewportWidth();
  float vp_graph_height = static_cast<float>(graphHeight) / getViewportHeight();
  float vp_text_height = static_cast<float>(fontSize) / getViewportHeight();

  std::stringstream min_ss, max_ss, current_ss;
  min_ss << std::fixed << std::setprecision(1) << current_msg_->min_value;
  max_ss << std::fixed << std::setprecision(1) << current_msg_->max_value;
  current_ss << std::fixed << std::setprecision(1) << current_msg_->current_value;

  min_text_->setCaption(Ogre::UTFString(min_ss.str()));
  min_text_->setFontSize(vp_text_height);
  min_text_->setColour(textColor);

  max_text_->setCaption(Ogre::UTFString(max_ss.str()));
  max_text_->setFontSize(vp_text_height);
  max_text_->setColour(textColor);

  graph_rect_->setMaterial("BaseWhiteNoLighting"); // Use a basic white material
  graph_rect_->setColour(Ogre::ColourValue(0.5f, 0.5f, 0.5f, 0.5f));

  float normalized_value = (current_msg_->current_value - current_msg_->min_value) / (current_msg_->max_value - current_msg_->min_value);
  normalized_value = std::max(0.0f, std::min(1.0f, normalized_value)); // Clamp to [0, 1]

  Ogre::ColourValue currentColor(current_msg_->current_color.r, current_msg_->current_color.g, current_msg_->current_color.b, current_msg_->current_color.a);

  if (orientation == 0) // Horizontal
  {
    float text_offset_x = vp_text_height * 3.0f; // Adjust as needed for spacing

    min_text_->setPosition(vp_x, vp_y + vp_graph_height / 2.0f - vp_text_height / 2.0f);
    max_text_->setPosition(vp_x + vp_graph_width + text_offset_x, vp_y + vp_graph_height / 2.0f - vp_text_height / 2.0f);
    graph_rect_->setPosition(vp_x + text_offset_x, vp_y);
    graph_rect_->setDimensions(vp_graph_width, vp_graph_height);

    float current_pos_x = vp_x + text_offset_x + normalized_value * vp_graph_width;

    if (currentValueDisplay == 0) // Line
    {
      current_line_->setPosition(current_pos_x - 0.001f, vp_y);
      current_line_->setDimensions(0.002f, vp_graph_height);
      current_line_->setColour(currentColor);
      current_line_->show();
      current_box