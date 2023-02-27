#include "Arduino.h"

const char HTTP_SETUP[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>%HTML_HEAD_TITLE%</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
<style>%CSS_TEMPLATE%</style>
</head>
<body class="invert">
  <div class="wrap">
    <div class="topnav">
      <h1>%HTML_HEAD_TITLE%</h1>
    </div>
    <div class="content">
    <p align="center">
      <img src="https://www.compuphase.com/electronics/solderprofile-standard.png" width="50%">
    </p>
      <div class="card-grid">
        <div class="card">
          <form action="/" method="POST">
            <p>
            <h3>Preheat</h3>
              <label for="s00">Setpoint &deg;C</label>
              <input type="number" id ="s00" name="s00" min="1" max="250" value=90 required><br>
              <label for="s01">Rate &deg;C/sec</label>
              <input type="number" id ="s01" name="s01" min="1" max="250" value=1 required><br>
            <h3>Soak</h3>
              <label for="s10">Setpoint &deg;C</label>
              <input type="number" id ="s10" name="s10" min="1" max="960" value=130 required><br>
              <label for="s11">Rate &deg;C/sec</label>
              <input type="number" id ="s11" name="s11" min="1" max="9999" value=2 required><br>
            <h3>Reflow</h3>
              <label for="s20">Setpoint &deg;C</label>
              <input type="number" id ="s20" name="s20" min="1" max="1240" value=138 required><br>
              <label for="s32">Hold sec</label>
              <input type="number" id ="s32" name="s32" min="0" max="3600" value=160 required><br>
              <input type ="submit" value ="FIRE">
            </p>
          </form>
        </div>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";
