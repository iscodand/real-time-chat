<script setup lang="ts">
import { ref, type Ref } from 'vue';


const url = "ws://localhost:3001/websocket";

const messages: Ref<string[]> = ref<string[]>([]);

const socket = new WebSocket(url);

const room = ref<string>("NÃO CONECTADO");

socket.onopen = (event) => {
  console.log("WebSocket connection opened:", event);
};

socket.onmessage = (event: MessageEvent<string>) => {
  messages.value.push(event.data);
  console.log(messages.value);
  console.log("message received: ", event.data)
}

type Request = {
  room: string,
  message: string
};

const request = ref<Request>({
  room: '',
  message: ''
});

async function sendMessage() {
  socket.send(request.value.message)
}

// TODO:
// 1. conectar o usuario a uma sala atraves de um input (DONE)

// bonus
// 1. tentar não perder a sessão no refresh (descobrir como fazer)

</script>

<template>
  <h1>SALA {{ request.room }}</h1>

  <!-- TODO: logic to put first message equals user room -->
  <div class="flex flex-col">
    <div class="col-span-1">
      <div>
        <label class="block text-gray-700 text-sm font-bold mb-2">Room</label>
        <input
          class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
          v-model="request.room" @keyup.enter="sendMessage" placeholder="Insira sua sala...">
        <button
          class="mt-5 bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded focus:outline-none focus:shadow-outline">Enviar</button>
      </div>
    </div>

    <div class="col-span-2 mt-10">
      <div>
        <label class="block text-gray-700 text-sm font-bold mb-2">Input Message</label>
        <input
          class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
          v-model="request.message" @keyup.enter="sendMessage" placeholder="Type a message...">
        <button
          class="mt-5 bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded focus:outline-none focus:shadow-outline">Enviar
          mensagem</button>
      </div>
    </div>

    <div v-for="(message, index) in messages" :key="index" class="message">
      {{ message }}
    </div>
  </div>
</template>