#ifndef OVERLAY_VALUE_DISPLAY_HPP_
#define OVERLAY_VALUE_DISPLAY_HPP_

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/subscription.hpp"
#include "overlay_value_msgs/msg/overlay_value_in_range.hpp"

#include "rviz_common/display.hpp"
#include "rviz_common/properties/property.hpp"
#include "rviz_common/properties/string_property.hpp"
#include "rviz_common/properties/float_property.hpp"
#include "rviz_common/properties/int_property.hpp"
#include "rviz_common/properties/color_property.hpp"
#include "rviz_common/properties/bool_property.hpp"
#include "rviz_common/properties/enum_property.hpp"

#include <OgreOverlayManager.h>
#include <OgreOverlay.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreRectangle2D.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreTechnique.h>
#include <OgreHardwarePixelBuffer.h>

namespace overlay_value_display
{

class OverlayValueDisplay : public rviz_common::Display
{
public:
  // Inherited from rviz_common::Display
  OverlayValueDisplay();
  virtual ~OverlayValueDisplay();

protected:
  // Inherited from rviz_common::Display
  void onInitialize() override;
  void onEnable() override;
  void onDisable() override;

  // Callback for the ROS topic
  void valueCallback(const overlay_value_msgs::msg::OverlayValueInRange::SharedPtr msg);

  // Update the overlay elements
  void updateOverlay();

  // Helper function to create or update a text area
  Ogre::TextAreaOverlayElement* createOrUpdateText(const std::string& name, const std::string& text, float x, float y, float w, float h);

  // Helper function to create or update a rectangle
  Ogre::Rectangle2D* createOrUpdateRect(const std::string& name, float x, float y, float w, float h, const Ogre::ColourValue& color);

  // ROS subscriber
  rclcpp::Subscription<overlay_value_msgs::msg::OverlayValueInRange>::SharedPtr value_sub_;

  // Ogre Overlay elements
  Ogre::Overlay* overlay_;
  Ogre::TextAreaOverlayElement* min_text_;
  Ogre::TextAreaOverlayElement* max_text_;
  Ogre::Rectangle2D* graph_rect_;
  Ogre::Rectangle2D* current_line_;
  Ogre::Rectangle2D* current_box_;
  Ogre::TextAreaOverlayElement* current_value_text_;

  // RViz properties
  rviz_common::properties::StringProperty* topic_property_;
  rviz_common::properties::ColorProperty* text_color_property_;
  rviz_common::properties::IntProperty* font_size_property_;
  rviz_common::properties::IntProperty* graph_width_property_;
  rviz_common::properties::IntProperty* graph_height_property_;
  rviz_common::properties::EnumProperty* display_orientation_property_;
  rviz_common::properties::IntProperty* overlay_x_property_;
  rviz_common::properties::IntProperty* overlay_y_property_;
  rviz_common::properties::EnumProperty* current_value_display_property_;
};

} // namespace overlay_value_display

#endif // OVERLAY_VALUE_DISPLAY_HPP_