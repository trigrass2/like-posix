
// update interval
var update_interval = null;
// update rate
var update_rate;
// update callback
var update_callback;
// update endpoint
var update_endpoint;
// update error callback
var update_error_callback;


function start_update(endpoint, callback, rate, onerror) {
  console.log("starting updater");
  update_endpoint = endpoint;
  update_callback = callback;
  update_rate = rate;
  if(onerror != null)
    update_error_callback = onerror;
  else
    update_error_callback = update_error_default;
  if(update_interval == null)
      update_interval = setInterval(get_update, rate);
}

function stop_update() {
  console.log("stopping updater");
  if(update_interval != null) {
    clearInterval(update_interval);
    update_interval = null;
  }
}

function update_error_default(jqXHR, textStatus, errorThrown) {
  console.log("update error: " + textStatus);
  stop_update();
}

function get_update() {
  $.ajax({url: update_endpoint,
  dataType: 'json',
  async: 'false',
  success: update_callback,
  error: update_error_callback,
  timeout: update_rate});
}
