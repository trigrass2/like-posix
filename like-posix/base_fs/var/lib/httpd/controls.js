

// sets the colour of a font awesome icon to green or grey
// id is the id of the font awesome icon, state may be "green" or "gray"
function set_green_led_color(id, state) {
  var item = $("#" + id);
  if(state === "green")
    item.css("color", "LimeGreen");
  else if(state === "gray")
    item.css("color", "#b8b8b8");
}