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

  if (TurnBellOff == true) {
    TurnBellOff = false;
    BellTurnOff();
  }


  check_btn();

  //  check_time();
  delay(100);

}
