int threshold_rise = 45;
int threshold_fall = 70;

int threshold_on_count = 0;
bool button_detected = false;
bool button_prev = false;
int button_update_timer = 0;
int button_long_timer = 0;
bool button_rose = false;
bool button_fell = false;
int button_long = false;
int button_long_prev = false;
int button_ignore_fell = false;
int button_super_long = false;
int button_super_long_prev = false;
bool button_pressed = false;

void buttonUpdate() {
  /*debouncer.update();
  if(debouncer.fell()) {
    button_pressed = true;
    button_prev = true;
    button_rose = true;
    button_fell = false;
    button_long_timer = millis();
    //Serial.println("Rose");
  }
  if (debouncer.rose()) {
    button_pressed = false;

    if (button_prev) {
      button_fell = true;
      button_rose = false;
    }
    button_prev = false;
    button_long_timer = millis();
    button_long = false;
    button_super_long_prev = false;
    button_long_prev = false;
    //Serial.println("Fell");
  }*/

  if (not button_pressed) button_long_timer = millis();

  if (millis() - button_long_timer > 1000 and not button_long_prev) {
    button_long = true;
    button_long_prev = true;
  }
  if (millis() - button_long_timer > 3000 and not button_super_long_prev) {
    button_super_long = true;
    button_super_long_prev = true;
  }
}

bool buttonRose() {
  if (button_rose) {
    button_rose = false;
    return true;
  }
  else {
    return false;
  }
}

bool buttonFell() {
  if (button_fell) {
    button_fell = false;
    if (button_ignore_fell) {
      button_ignore_fell = false;
      return false;
    }
    return true;
  }
  else {
    return false;
  }
}

bool buttonLong() {
  if (button_long) {
     Serial.println("Long");
    button_long = false;
    button_ignore_fell = true;
    return true;
  }
  else {
    return false;
  }
}

bool buttonSuperLong() {
  if (button_super_long) {
     Serial.println("Super Long");
    //We want to always return true, this is imminent deep sleep command
    button_super_long = false;
    button_ignore_fell = false;
    return true;
  }
  else {
    return false;
  }
}
