#include "util_app.h"

App available_apps[] = {
  {"ASR", ASR_init, NULL, NULL, NULL, ASR_resume,
    _loop, ASR_menu, screensaver, topButton, lowerButton, rightButton, leftButton, NULL, update_ENC},
  {"Harrington 1200", H1200_init, NULL, NULL, NULL, H1200_resume,
    H1200_loop, H1200_menu, H1200_screensaver, H1200_topButton, H1200_lowerButton, H1200_rightButton, H1200_leftButton, NULL, H1200_encoders},
  {"Automatonnetz", Automatonnetz_init, NULL, NULL, NULL, Automatonnetz_resume,
    Automatonnetz_loop, Automatonnetz_menu, Automatonnetz_screensaver, Automatonnetz_topButton, Automatonnetz_lowerButton, Automatonnetz_rightButton, Automatonnetz_leftButton, Automatonnetz_leftButtonLong, Automatonnetz_encoders},
  {"VierfStSpQuaMo", QQ_init, NULL, NULL, NULL, QQ_resume,
    QQ_loop, QQ_menu, screensaver, QQ_topButton, QQ_lowerButton, QQ_rightButton, QQ_leftButton, QQ_leftButtonLong, QQ_encoders}
};

struct global_app_settings {
  static const uint32_t FOURCC = FOURCC<'O', 'C', 0, 9>::value;

  uint8_t current_app_index;
};

#define EEPROM_CALIBRATIONDATA_START 0
#define EEPROM_CALIBRATIONDATA_LENGTH 64 // calibrate.ini: OCTAVES*uint16_t + numADC*unit16_t = 14 * 2 = 28 -> leaves space

global_app_settings app_settings;
PageStorage<EEPROMStorage, EEPROMStorage::LENGTH - 128, EEPROMStorage::LENGTH, global_app_settings> settings_storage;

static const int APP_COUNT = sizeof(available_apps) / sizeof(available_apps[0]);
App *current_app = &available_apps[0];
bool SELECT_APP = false;
static const uint32_t SELECT_APP_TIMEOUT = 15000;
static const int DEFAULT_APP_INDEX = 1;

void draw_app_menu(int selected) {
  u8g.setFont(u8g_font_6x12);
  u8g.firstPage();
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  u8g.firstPage();  
  do {
    uint8_t x = 4, y = 0;
    for (int i = 0; i < APP_COUNT; ++i, y += 16) {
      if (i == selected) {
        u8g.setDefaultForegroundColor();
        u8g.drawBox(0, y, 128, 15);
        u8g.setDefaultBackgroundColor();
      } else {
        u8g.setDefaultForegroundColor();
      }

      u8g.setPrintPos(x, y + 2);
      if (app_settings.current_app_index == i)
        u8g.print('>');
      else
        u8g.print(' ');
      u8g.print(available_apps[i].name);
    }
  } while (u8g.nextPage());
}

void set_current_app(int index) {
  app_settings.current_app_index = index;
  current_app = &available_apps[index];
}

void init_apps() {
  for (int i = 0; i < APP_COUNT; ++i)
    available_apps[i].init();

  if (!settings_storage.load(app_settings) || app_settings.current_app_index >= APP_COUNT) {
    app_settings.current_app_index = DEFAULT_APP_INDEX;
  } else {
    Serial.print("Loaded settings... ");
    Serial.println(app_settings.current_app_index);
  }
  set_current_app(app_settings.current_app_index);
  if (current_app->resume)
    current_app->resume();

  if (!digitalRead(but_top)) {
    set_current_app(0);
    draw_app_menu(0);
    while (!digitalRead(but_top));
  } else if (!digitalRead(but_bot)) {
    set_current_app(2);
    draw_app_menu(2);
    while (!digitalRead(but_bot));
  } else if (!digitalRead(butR)) {
    select_app();
  } else {
    delay(500);
  }
}

void select_app() {

  // Save state
  int encoder_values[2] = { encoder[LEFT].pos(), encoder[RIGHT].pos() };
  if (current_app->suspend)
    current_app->suspend();

  int selected = app_settings.current_app_index;
  encoder[RIGHT].setPos(selected);

  draw_app_menu(selected);
  while (!digitalRead(butR));

  uint32_t time = millis();
  bool redraw = true;
  bool save = false;
  while (!(millis() - time > SELECT_APP_TIMEOUT)) {
    if (_ENC) {
      _ENC = false;
      int value = encoder[RIGHT].pos();
      if (value < 0) value = 0;
      else if (value >= APP_COUNT) value = APP_COUNT - 1;
      encoder[RIGHT].setPos(value);
      if (value != selected) {
        selected = value;
        redraw = true;
      }
    }
    button_right.read();
    if (button_right.long_event()) {
      save = true;
      break;
    }
    else if (button_right.event())
      break;

    if (redraw) {
      draw_app_menu(selected);
      redraw = false;
    }
  }

  set_current_app(selected);
  if (save) {
    Serial.println("Saving settings...");
    settings_storage.save(app_settings);
  }

  // Restore state
  encoder[LEFT].setPos(encoder_values[0]);
  encoder[RIGHT].setPos(encoder_values[1]);

  if (current_app->resume)
    current_app->resume();

  SELECT_APP = false;
  MENU_REDRAW = 1;
  _UI_TIMESTAMP = millis();
}
