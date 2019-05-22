// använder mosca
var mosca = require('mosca');
// porten som ska skicka och som ska hämta
var settings={
    port:1883,
    http:{
        port:1884,
        bundle:true,
        static:'./'
    }
}   // 
var server = new mosca.Server(settings);
// skiver ut redo när allt är kopplat
server.on('ready',function(){
    console.log("redo!");
})
// skriver ut connect om den kunde koppla 
server.on('clientConnected',function(client){
    console.log("Client connected! ", client.id);
})

server.on('published',function(packet){
})