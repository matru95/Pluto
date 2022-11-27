const api_url = "https://roboticsanddesign.herokuapp.com/";
const loader = document.getElementById("modalLoader");
var ready = true;


function redirect_to_museum(){
    window.location.href = "https://www.museoscienza.org/it/offerta/esposizioni-permanenti/spazio";
}

async function fetch_api(url) { 
    const response = await fetch(url, {
        method: 'GET'
    });
    let result = await response.json();
    return result;
}


async function send_command(url, command) { 
    const response = await fetch(url, {
        method: 'POST',
        mode: 'cors',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({command: command})
    });
    return response;
}


async function set_command(command){
    if(ready){
        loader.style.display = "block";
        ready = false;
        let response = await fetch_api(api_url);
    
        let message = document.getElementById("message");
        let messageText = document.getElementById("messageText");
        let messageImg = document.getElementById("messageImg");
        if(response.status == "busy"){
            loader.style.display = "none";

            message.style.backgroundColor = "#D75A4A";
            messageText.innerHTML = "Sorry, the robot is still executing an action. Please try again later.";
            messageImg.src = "images/warning.svg";
    
            message.style.display = "block";
            message.classList.remove('slideUp');
            message.classList.add('slideDown');
        
            setTimeout(function(){
                message.classList.remove('slideDown');
                message.classList.add('slideUp');
                setTimeout(function(){
                    message.style.display = "none";
                }, 1000);
            }, 4500);
        }
        else if(response.status == "free"){ 
            send_command(api_url, command).then(result =>{
                if(result.status == 200){
                    loader.style.display = "none";
                    message.style.backgroundColor = "#25AE88";
                    messageText.innerHTML = "The command was sent to the robot successfully.";
                    messageImg.src = "images/success.svg";

                    message.style.display = "block";
                    message.classList.remove('slideUp');
                    message.classList.add('slideDown');
                    
                    let text = document.getElementsByClassName("buttonText")[command - 1];
                    text.classList.add("showText");

                
                    setTimeout(function(){
                        message.classList.remove('slideDown');
                        message.classList.add('slideUp');
                        setTimeout(function(){
                            message.style.display = "none";
                        }, 1000);
                    }, 4500);
                }
            })
        }

        setTimeout(function(){
            ready = true;
        },5500);
    }
}