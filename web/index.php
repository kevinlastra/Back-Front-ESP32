<?php
session_start();
?>

<!DOCTYPE html>
<head>
  <meta charset="UTF-8">
  <title>SESIBG</title>
  <link rel="stylesheet" href="main.css" type="text/css">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css"/>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
</head>
<body>
  <nav>
    <label class="logo">SESIBG</label>
  </nav>
  <div style="height: 80px; width: 100%;">
    <div style="width: 30%; float: left;">
      LED
      </br>
      <div class="toggle-container">
          <input type="checkbox" id="checkbox" onchange="submit_led()"
          <?php if ($status == 1) { echo " checked='checked'"; }?>>     
          <div class="slider round"></div>
        </div>
    </div>
    <div style="width: 30%; float: left;">
       Get Button value
      </br>
      <div class="toggle-container">
        <input type="checkbox" id="btn_checkbox" onchange="submit_btn()" <?php if($btn_checked == 1) {echo " checked='checked'";}?>>
        <div class="slider round"></div>
      </div>
    </div>
    <div style="width: 30%; float: left;">
      Get Photo-sensor value
      </br>
      <div class="toggle-container">
        <input type="checkbox" id="ls_checkbox" onchange="submit_ls()" <?php if($ls_checked == 1) {echo " checked='checked'";}?>>
        <div class="slider round"></div>
      </div>
    </div>
  </div>
  <div style="height: 100px; width: 100%;" id="stats">
    <label id="btnstats"></label>
    <label id="lsstats"></label>
  </div>
  <div>
    <div style="float: left; width: 80%;">  
      <input type="text" id="oled_textarea" name="action2" rows="4" cols="50" >
      <button onclick="submit_text()">Write</button>
    </div>
  </div>
  </br>
  

  <script src="script.js"></script>

</body>
