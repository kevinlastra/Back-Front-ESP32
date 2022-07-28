setInterval(function() {
    if (document.getElementById('btn_checkbox').checked)
    {
        fetch("btn.json")
        .then((response) => {
            return response.json();
        })
        .then((data) => {
            var html = "<label>Button state:  "+data.state+"</label>";
            document.getElementById('btnstats').innerHTML = html;
        });
    }
    else
    {
        document.getElementById('btnstats').innerHTML = "";
    }
    if (document.getElementById('ls_checkbox').checked)
    {
        fetch("ls.json")
        .then((response) => {
            return response.json();
        })
        .then((data) => {
            var html = "</br><label>Photo sensor value: "+data.value+"</label>";
            document.getElementById('lsstats').innerHTML = html;
        });
    }  
    else
    {
        document.getElementById('lsstats').innerHTML = "";
    }  
}, 2000);


function submit_led()
{
    console.log("led");
    var val = document.getElementById("checkbox").checked;
    var formData = new FormData();
    formData.append('led',val);
    fetch("write_pipe.php", {method: 'POST', body: formData})
    .then(function (response){
        console.log(response);
    });
}
function submit_btn()
{
    console.log("btn");
    var val = document.getElementById("btn_checkbox").checked;
    var formData = new FormData();
    formData.append('btn',val);
    fetch("write_pipe.php", {method: 'POST', body: formData})
    .then(function (response){
        console.log(response);
    });
}
function submit_ls()
{
    console.log("ls");
    var val = document.getElementById("ls_checkbox").checked;
    var formData = new FormData();
    formData.append('ls',val);
    fetch("write_pipe.php", {method: 'POST', body: formData})
    .then(function (response){
        console.log(response);
    });
}
function submit_text()
{
    console.log("lcd");
    var val = document.getElementById("oled_textarea").value;
    var formData = new FormData();
    formData.append('lcd',val);
    fetch("write_pipe.php", {method: 'POST', body: formData})
    .then(function (response){
        console.log(response);
    });
}