<?php
$pipe = "s2c_LG";
if(!file_exists($pipe))
{
    $mode = 0600;
    posix_mkfifo($pipe, $mode);
}
$myfile = fopen($pipe, "w");

$get = "";

if(isset($_POST['led']))
{
    $get = $_POST['led'];
    if($get == "true")
        $get = "LEDON";
    else
        $get = "LEDOFF";
}
else if(isset($_POST['btn']))
{
    $get = $_POST['btn'];
    if($get == "true")
        $get = "BTNON";
    else
        $get = "BTNOFF";
}
else if(isset($_POST['ls']))
{
    $get = $_POST['ls'];
    if($get == "true")
        $get = "LSON";
    else
        $get = "LSOFF";
}
else if(isset($_POST['lcd']))
{
    $get = "LCD#".$_POST['lcd'];
}

if($get != "")
    fwrite($myfile, $get);

fclose($myfile);
?>