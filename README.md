# Fernairon-Hordas

Ver. 0.1.6:
 -Reasignada la tecla para recargar a "R"
 -Añadida animacion para recargar a las IAs
 -Añadido pack de armas gratuito
 -Creación del AK-47
 -Creación del Lanza pepos MBG1
 -Creado un WBP (Widget blueprint) para etiquetar los enemigos
 -Creado un WBP para dar información sobre el arma en el suelo (Proximamente podrás equipartela)
 -Añadido una clase C++ "SUsableActor" encargada de administrar las interacciones con los objetos del suelo
 -Reasignado el padre de la clase C++ "SWeapon" a "SUsableActor"
 -Añadido la clase Blueprint "Bullet"
 -Añadido un socket al AK-47 con la clase "Bullet" asignado a él
 -Añadido la animación de escupir los casquillos de las balas al disparar con el AK-47
 -Añadido la clase Blueprint "Clip"
 -Añadido un socket correspondiente con la clase "Clip" asignado a él
 -Añadida la animación de "Soltar cargador" al recargar (Proximamente se añadirá la animación de poner cargador)
 -Implemetada la munición
 -Arreglado varios bug de las IA


Ver. 0.1.5:
 -Añadida animacion para recargar (tecla U)
 -Mejorada la interaccion entre animaciones
 -Implementado la munición (No añadido a la UI)
 -Arreglado un bug del lanzagranadas
 -Arreglado varios bug de las IA
 

Instrucciones y requisitos:

- Unreal Engine Ver. 4.20
- Visual Studio 2017 última versión.

+ Descargar el proyecto
+ Al abrirlo no estará montado el codigo C++, para hacerlo ir a la opción superior izquierda,  "Archivo (file)" -> "Create visual studio project".
+ Tendreis que activar EQS para que la IA vaya correctamente, siguiendo los siguientes pasos:
  - Edit-> EditorPreferences -> en la ventana que se os abrirá en la categoría "General" de la izquierda hacer click en "experimental"-> En la sección "AI" activar la casilla de "Environment Query System".
+ Si no os gusta que al darle al "Play" se os abra el juego en una ventana aparte, darle a la flechita a la derecha del play y escoger la opción "Selected Viewport"


 **Queda terminantemente prohibido comercializar el código y resto del proyecto residido en este repositorio.**
