void loop()
{

  // Check Network only, if Bell not started
  if (BellStarted == false) {

    if (gv_check_net == true) {
      check_ota();

      // check_mqtt();
      check_mqtt_reset();

      gv_check_net = false;
    }

  }

  check_bell( );

  check_btn();

  delay(100);

}
