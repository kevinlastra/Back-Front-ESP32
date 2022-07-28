<?php
    $pipe = "c2s_btn_LG";
    if(!file_exists($pipe))
    {
        $mode = 0600;
        posix_mkfifo($pipe, $mode);
    }
    $myfile = fopen($pipe, "w");

    $get = fread($myfile,10);
    echo $get;
    close($myfile);
?>