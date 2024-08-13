window.onload = () => {
    const message = document.getElementById("message");
    const messages = document.getElementById("messages");
    let ws = new WebSocket('ws://127.0.0.1:3001/websocket');

    if ('WebSocket' in window) {
        ws.onopen = () => {
            console.log('WebSocket connection established');
        }

        ws.onmessage = (event) => {
            let data = event.data;
            messages.innerHTML = data;
            console.log('Received WebSocket message:', data);
        }

        ws.onerror = () => {
            console.error('WebSocket error');
        }

        ws.onclose = () => {
            console.log('WebSocket connection closed');
        }

        document.querySelector("button").addEventListener("click", () => {
            sendMyMessages();
        });
    } else {
        console.error('WebSocket not supported');
    }

    function sendMyMessages() {
        if (ws.readyState === WebSocket.OPEN) {
            const messageValue = message.value;
            console.log("Sending message:", messageValue);
            ws.send(messageValue);
        } else {
            console.error('WebSocket is not open. ReadyState: ' + ws.readyState);
        }
    }
};
