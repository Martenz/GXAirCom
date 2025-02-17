#include <BeeperTzi.h>
#include <melodies.h>

Buzzer::Buzzer(){
}

uint32_t Buzzer::interpolate(StaticJsonDocument<1024> curves, float vval, const char *field){
  
  uint32_t interpolated = 0;
  int s = sizeof(curves["vario_curve"]);
  float v1 = 0.;
  float v2 = 0.;
  uint32_t f1 = 0;
  uint32_t f2 = 0;
  bool minFound = false;
  bool maxFound = false;
  for (int i =0;i<s;i++){
    float v = curves["vario_curve"][i]["vval"];
    if (vval < v && !minFound){
      v1 = v;
      f1 = curves["vario_curve"][i][field];
    }else{ minFound = true;}
    if (vval > v && !maxFound){
      v2 = v;
      f2 = curves["vario_curve"][i][field];
      maxFound = true;
    }
    if (v == vval){
      return curves["vario_curve"][i][field];
    }
  }
  float ratio = (vval - v1)/(v2 - v1);
  int sign = (float)f2 - (float)f1 >= 0 ? 1 : -1;
  interpolated = f1 + sign * ratio * abs(((float)f2 - (float)f1)); 

  return interpolated;
}

void Buzzer::sing(uint8_t spk_pin, int song) {
  // iterate over the notes of the melody:
    ledcAttachPin(spk_pin, LEDC_CHANNEL_0);

    int size = sizeof(melody[song]) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      if (tempo[song][thisNote] == 0) break;
        // to calculate the note duration, take one second
        // divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int noteDuration = 1000 / tempo[song][thisNote];

        ledcWriteTone(LEDC_CHANNEL_0, melody[song][thisNote]);
        ledcWrite(LEDC_CHANNEL_0, setting.vario.volume);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        ledcWriteTone(LEDC_CHANNEL_0, 0);
//        delay(noteDuration);
    }

    ledcDetachPin(spk_pin);
}

bool Buzzer::begin(uint8_t spk_pin){

  ledcSetup(LEDC_CHANNEL_0, 1000, 8);
  ledcAttachPin(spk_pin, LEDC_CHANNEL_0);
  bool von = false;
  uint32_t tsound = millis();
  StaticJsonDocument<1024> vario_curve;
  DeserializationError error = deserializeJson(vario_curve, setting.vario.varioCurve);

  delay(2000);
  sing(spk_pin,0);
  delay(3000);
  return true;
}

void Buzzer::end(uint8_t spk_pin){
  ledcDetachPin(spk_pin);
}

void Buzzer::run(uint8_t spk_pin){
  int fr = 0;
  uint32_t son = 50; //ms
  uint32_t soff = 50; //ms

  // if volume is 0 then do not beep
  if (setting.vario.volume == 0) von=false;

  if (setting.vario.volume> 0 && von == false){
    von = true;
    ledcSetup(LEDC_CHANNEL_0, 1000, 8);
    ledcAttachPin(spk_pin, LEDC_CHANNEL_0);
  }
  if (von){

    fr = interpolate(vario_curve, status.vario.ClimbRate, "frq");
    son = interpolate(vario_curve, status.vario.ClimbRate, "ton");
    soff = interpolate(vario_curve, status.vario.ClimbRate, "toff");

    // TODO - Add test vario to play with web interface
    // if (status.test_vario){
    //   fr = status.tv_frq;
    //   son = status.tv_ton;
    //   soff = status.tv_toff;
    // }

    bool beep = (!setting.vario.BeepOnlyWhenFlying);
    beep = beep || setting.vario.BeepOnlyWhenFlying && status.flying;
    beep = beep && (status.vario.ClimbRate <=  setting.vario.sinkingThreshold || status.vario.ClimbRate >= setting.vario.climbingThreshold);
    // TODO test if playing with web interface
    bool beepTest = false;//status.test_vario && (status.tv_vval <= setting.vario.sinkingThreshold || status.tv_vval >= setting.vario.climbingThreshold);
    
    if (beep || beepTest){
        ledcWriteTone(LEDC_CHANNEL_0, fr);
        ledcWrite(LEDC_CHANNEL_0, setting.vario.volume);
        tsound = millis();
        while (millis() - tsound > son){
          ledcWriteTone(LEDC_CHANNEL_0, 0);
          ledcWrite(LEDC_CHANNEL_0, 0);
        }
    }else{
        ledcWriteTone(LEDC_CHANNEL_0, 0);
        ledcWrite(LEDC_CHANNEL_0, 0);
      }
    
  }else{
    ledcDetachPin(spk_pin);
  }

  // TODO Add test speaker single beep check
  // if (status.test_speaker){
  //   beepSpeaker();
  //   status.test_speaker = false;
  // }

  delay(soff);
}
