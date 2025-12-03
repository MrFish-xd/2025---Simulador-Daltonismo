# 2025---Simulador-Daltonismo
Nuestra idea, en conjunto con las chicas de óptica, era lograr recrear cómo ven las personas con diferentes discromatopsias (trastorno de la visión que afecta la capacidad de distinguir o percibir colores) de los conos, y esto se mostraría en una página web en donde se pueden cambiar los diferentes filtros.

Esto lo conseguimos con:

· ESP32-CAM: Es la encargada de prácticamente todo el simulador; nos proporciona el video, se conecta al WiFi y es donde está cargado el código.

· Arduino UNO: Es el encargado de proporcionar energía y de cargar el código a la ESP32-CAM. También se encarga de recibir los datos de las acciones realizadas por los usuarios y mostrarlos en Arduino IDE.

· Servo de 180º: Es el encargado de mover la cámara, simulando una visión periférica. Este es controlado por la página web para que los usuarios puedan ver el entorno.

 Creado por:
 - Jeremias Lopez
 - León Rivadero
 - Amaya Florencia
 - González Agostina
 - Santos Gisella
 - Quispe Maribel 

