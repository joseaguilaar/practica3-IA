# include "AIPlayer.h"
# include "../../include/model/Parchis.h"

const float masinf = 9999999999.0, menosinf = -9999999999.0;
const float gana = masinf / 10.f, pierde = menosinf / 10.f;
const int num_pieces = 2;
const int PROFUNDIDAD_MINIMAX = 4; // Umbral maximo de profundidad para el metodo MiniMax
const int PROFUNDIDAD_ALFABETA = 8; // Umbral maximo de profundidad para la poda Alfa_Beta

bool AIPlayer::move(){
   cout << COUT_ORANGE_BOLD << "Realizo un movimiento automatico" << COUT_NOCOLOR << endl;

   color c_piece;
   int id_piece;
   int dice;
   think(c_piece, id_piece, dice);

   cout << COUT_ORANGE_BOLD << "Movimiento elegido: " << str(c_piece) << " " << id_piece << " " << dice << COUT_NOCOLOR << endl;

   actual->movePiece(c_piece, id_piece, dice);
   return true;
}

void AIPlayer::think(color& c_piece, int& id_piece, int& dice) const
{
   
   // El siguiente código se proporciona como sugerencia para iniciar la implementación del agente.

   float valor; // Almacena el valor con el que se etiqueta el estado tras el proceso de busqueda.
   float alpha = menosinf, beta = masinf; // Cotas iniciales de la poda AlfaBeta
   // // Llamada a la función para la poda (los parámetros son solo una sugerencia, se pueden modificar).
   ValoracionTest valoracionTest;
   Heuristica1 heuristica1;

   // ----------------------------------------------------------------- //
   switch (id)
   {
      case 0:
      valor = Poda_AlfaBeta(*actual, jugador, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, &heuristica1);
      break;

      case 1:
      valor = ProfundidadDinamica(*actual, jugador, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, &valoracionTest);
      break;

      case 2:
      valor = OrdenacionMovimientos(*actual, jugador, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, alpha, beta, &heuristica1);
      break;

      case 3:
      break;
      
   }
      cout << "Valor MiniMaxLimitado: " << valor << " Accion: " << str(c_piece) << " " << id_piece << " " << dice << endl;
}


void AIPlayer::thinkAleatorio(color &c_piece, int &id_piece, int &dice) const
{

   // IMPLEMENTACIÓN INICIAL DEL AGENTE
   // Esta implementación realiza un movimiento aleatorio.
   // Se proporciona como ejemplo, pero se debe cambiar por una que realice un movimiento inteligente
   //como lo que se muestran al final de la función.

   // OBJETIVO: Asignar a las variables c_piece, id_piece, dice (pasadas por referencia) los valores,
   //respectivamente, de:
   // - color de ficha a mover
   // - identificador de la ficha que se va a mover
   // - valor del dado con el que se va a mover la ficha.

   // El id de mi jugador actual.
   int player = actual->getCurrentPlayerId();

   // Vector que almacenará los dados que se pueden usar para el movimiento
   vector<int> current_dices;
   // Vector que almacenará los ids de las fichas que se pueden mover para el dado elegido.
   vector<tuple<color, int>> current_pieces;

   // Se obtiene el vector de dados que se pueden usar para el movimiento
   current_dices = actual->getAvailableNormalDices(player);
   // Elijo un dado de forma aleatoria.
   dice = current_dices[rand() % current_dices.size()];

   // Se obtiene el vector de fichas que se pueden mover para el dado elegido
   current_pieces = actual->getAvailablePieces(player, dice);

   // Si tengo fichas para el dado elegido muevo una al azar.
   if (current_pieces.size() > 0){
      int random_id = rand() % current_pieces.size();
      id_piece = get<1>(current_pieces[random_id]); // get<i>(tuple<...>) me devuelve el i-ésimo
      c_piece = get<0>(current_pieces[random_id]); // elemento de la tupla
   }
   else{
      // Si no tengo fichas para el dado elegido, pasa turno (la macro SKIP_TURN me permite no mover).
      id_piece = SKIP_TURN;
      c_piece = actual->getCurrentColor(); // Le tengo que indicar mi color actual al pasar turno.
   }
}

void AIPlayer::thinkFichaMasAdelantada(color &c_piece, int &id_piece, int &dice) const
{
   // Elijo el dado haciendo lo mismo que el jugador aleatorio.
   thinkAleatorio(c_piece, id_piece, dice);

   // Tras llamar a esta función, ya tengo en dice el número de dado que quiero usar.
   // Ahora, en vez de mover una ficha al azar, voy a mover (o a aplicar
   // el dado especial a) la que esté más adelantada
   // (equivalentemente, la más cercana a la meta).
   int player = actual->getCurrentPlayerId();
   vector<tuple<color, int>> current_pieces = actual->getAvailablePieces(player, dice);
   int id_ficha_mas_adelantada = -1;
   color col_ficha_mas_adelantada = none;
   int min_distancia_meta = 9999;
   
   
   for (int i = 0; i < current_pieces.size(); i++)
   {
      // distanceToGoal(color, id) devuelve la distancia a la meta de la ficha [id] del color que le indique.
      color col = get<0>(current_pieces[i]);
      int id = get<1>(current_pieces[i]);
      int distancia_meta = actual->distanceToGoal(col, id);

      if (distancia_meta < min_distancia_meta)
      {
         min_distancia_meta = distancia_meta;
         id_ficha_mas_adelantada = id;
         col_ficha_mas_adelantada = col;
      }
   }

   // Si no he encontrado ninguna ficha, paso turno.
   if (id_ficha_mas_adelantada == -1)
   {
      id_piece = SKIP_TURN;
      c_piece = actual->getCurrentColor(); // Le tengo que indicar mi color actual al pasar turno.
   }
   // En caso contrario, moveré la ficha más adelantada.
   else
   {
      id_piece = id_ficha_mas_adelantada;
      c_piece = col_ficha_mas_adelantada;
   }
}

void AIPlayer::thinkMejorOpcion(color &c_piece, int &id_piece, int &dice) const
{
   // Vamos a mirar todos los posibles movimientos del jugador actual accediendo a los hijos del estado actual.
   // Cuando ya he recorrido todos los hijos, la función devuelve el estado actual. De esta forma puedo saber
   // cuándo paro de iterar.
   // Para ello, vamos a iterar sobre los hijos con la función de Parchis getChildren().
   // Esta función devuelve un objeto de la clase ParchisBros, que es una estructura iterable
   // sobre la que se pueden recorrer todos los hijos del estado sobre el que se llama.
   ParchisBros hijos = actual->getChildren();

   bool me_quedo_con_esta_accion = false;
   
   // La clase ParchisBros viene con un iterador muy útil y sencillo de usar.
   // Al hacer begin() accedemos al primer hijo de la rama,
   // y cada vez que hagamos ++it saltaremos al siguiente hijo.
   // Comparando con el iterador end() podemos consultar cuándo hemos terminado de visitar los hijos.
   for (ParchisBros::Iterator it = hijos.begin(); it != hijos.end() and !me_quedo_con_esta_accion; ++it)
   {
      Parchis siguiente_hijo = *it;
      // Si en el movimiento elegido comiera ficha, llegara a la meta o ganara, me quedo con esa acción.// Termino el bucle en este caso.
      if (siguiente_hijo.isEatingMove() or   siguiente_hijo.isGoalMove() or
          (siguiente_hijo.gameOver() and siguiente_hijo.getWinner() == this->jugador))
      {
         me_quedo_con_esta_accion = true;
         c_piece = it.getMovedColor();
         id_piece = it.getMovedPieceId();
         dice = it.getMovedDiceValue();
      }
   }
   // Si he encontrado una acción que me interesa, la guardo en las variables pasadas por referencia.
   // (Ya lo he hecho antes, cuando les he asignado los valores con el iterador).
   // Si no he encontrado ninguna acción que me interese, voy a mover la ficha más adelantada como en el caso anterior 
   if(!me_quedo_con_esta_accion){
      thinkFichaMasAdelantada(c_piece, id_piece, dice);
   }

}

float AIPlayer::Minimax(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, Heuristic *heuristic) const
{
   if (profundidad == profundidad_max || actual.gameOver())
   {  // Nodo terminal o profundidad límite: llamo a la función heurística
      // IMPORTANTE: USAMOS EL MÉTODO evaluate AUNQUE HAYAMOS REDEFINIDO LA CLASE HEURISTIC
      return heuristic->evaluate(actual, jugador);
   }
   // Comparo mi jugador con el actual de la partida para saber en qué tipo de nodo estoy
   else if (actual.getCurrentPlayerId() == jugador)
   { 
      // Nodo MAX
      float valor = menosinf; // Inicialización lo más pequeña posible para ir calculando el máximo

      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();

      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;

         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Minimax(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice,heuristic);
         if (new_val > valor)
         {
            // Me voy quedando con el máximo
            valor = new_val;
            if (profundidad == 0)
            {
               // Almaceno el movimiento que me ha llevado al mejor valor (solo en la raíz)
               c_piece = it.getMovedColor();
               id_piece = it.getMovedPieceId();
               dice = it.getMovedDiceValue();
            }
         }
      }
      return valor;
   }
   else
   { 
      // Nodo MIN
      float valor = masinf; // Inicialización lo más grande posible para ir calculando el mínimo

      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();

      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;
         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Minimax(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, heuristic);
         // Me voy quedando con el mínimo
         if (new_val < valor)
            valor = new_val;
      }

      return valor;
   }
}

float AIPlayer::Minimax_Limitado(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, Heuristic *heuristic) const
{

   if (profundidad == profundidad_max || actual.gameOver())
   { // Nodo terminal o profundidad límite: llamo a la función heurística
      // IMPORTANTE: USAMOS EL MÉTODO evaluate AUNQUE HAYAMOS REDEFINIDO LA CLASE HEURISTIC
      return heuristic->evaluate(actual, jugador);
   }
   else if (actual.getCurrentPlayerId() == jugador)   // Nodo MAX
   { 
      float valor = menosinf;

      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();
      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;

         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            if(profundidad == 0)
            {
               c_piece = it.getMovedColor();
               id_piece = it.getMovedPieceId();
               dice = it.getMovedDiceValue();
            }
            return valor;
         }

         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Minimax_Limitado(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, heuristic);

         if (new_val > valor)
         {
            // Me voy quedando con el máximo
            valor = new_val;
            if (profundidad == 0)
            {
               // Almaceno el movimiento que me ha llevado al mejor valor (solo en la raíz)
               c_piece = it.getMovedColor();
               id_piece = it.getMovedPieceId();dice = it.getMovedDiceValue();
               dice = it.getMovedDiceValue();
            }
         }
      }

      return valor;
   }
   else   // Nodo MIN
   { 
      float valor = masinf;

      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();
      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;
         
         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            return heuristic->evaluate(actual, jugador);
         }
         
         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Minimax_Limitado(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, heuristic);
         
         // Me voy quedando con el mínimo
         if (new_val < valor)
         {
            valor = new_val;
         }
      }

      return valor;
   }
}

float AIPlayer::Minimax_Limitado_Nodos(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, Heuristic *heuristic) const
{

   if (profundidad == profundidad_max || actual.gameOver())
   { // Nodo terminal o profundidad límite: llamo a la función heurística
      // IMPORTANTE: USAMOS EL MÉTODO evaluate AUNQUE HAYAMOS REDEFINIDO LA CLASE HEURISTIC
      return heuristic->evaluate(actual, jugador);
   }
   else if (actual.getCurrentPlayerId() == jugador)   // Nodo MAX
   { 
      float valor = menosinf;

      // Obtengo los hijos del nodo actual y los recorro
      vector<ParchisSis> rama = actual.getChildrenList();      
      
      for(int i = 0; i < rama.size(); i++){
         
         ParchisSis hijo_i = rama[i];     // Acceso al estado con el hijo i-ésimo del vector
         Parchis nuevo_hijo = *hijo_i;    // Acceso al estado
                  

         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            if(profundidad == 0)
            {
               c_piece = hijo_i.getMovedColor();
               id_piece = hijo_i.getMovedPieceId();
               dice = hijo_i.getMovedDiceValue();
            }
            return valor;
         }

         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Minimax_Limitado(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, heuristic);

         if (new_val > valor)
         {
            // Me voy quedando con el máximo
            valor = new_val;
            if (profundidad == 0)
            {
               // Almaceno el movimiento que me ha llevado al mejor valor (solo en la raíz)
               c_piece = hijo_i.getMovedColor();
               id_piece = hijo_i.getMovedPieceId();
               dice = hijo_i.getMovedDiceValue();
            }
         }
      }

      return valor;
   }
   else   // Nodo MIN
   { 
      float valor = masinf;

      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();
      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;
         
         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            return heuristic->evaluate(actual, jugador);
         }
         
         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Minimax_Limitado(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, heuristic);
         
         // Me voy quedando con el mínimo
         if (new_val < valor)
         {
            valor = new_val;
         }
      }

      return valor;
   }
}


float ValoracionTest::getHeuristic(const Parchis& estado, int jugador) const{
   // Heurística de prueba proporcionada para validar el funcionamiento del algoritmo de búsqueda.


   int ganador = estado.getWinner();
   int oponente = (jugador + 1) % 2;

   // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
   if (ganador == jugador){
      return gana;
   }
   else if (ganador == oponente){
      return pierde;
   }
   else{
      // Colores que juega mi jugador y colores del oponente
      vector<color> my_colors = estado.getPlayerColors(jugador);
      vector<color> op_colors = estado.getPlayerColors(oponente);

      // Recorro todas las fichas de mi jugador
      int puntuacion_jugador = 0;
      // Recorro colores de mi jugador.
      for (int i = 0; i < my_colors.size(); i++){
         color c = my_colors[i];
         // Recorro las fichas de ese color.
         for (int j = 0; j < num_pieces; j++){
            // Valoro positivamente que la ficha esté en casilla segura o meta.
            if (estado.isSafePiece(c, j)){
               puntuacion_jugador++;
            }
            else if (estado.getBoard().getPiece(c, j).get_box().type == goal){
               puntuacion_jugador += 5;
            }
         }
      }

      // Recorro todas las fichas del oponente
      int puntuacion_oponente = 0;
      // Recorro colores del oponente.
      for (int i = 0; i < op_colors.size(); i++){
         color c = op_colors[i];
         // Recorro las fichas de ese color.
         for (int j = 0; j < num_pieces; j++){
            if (estado.isSafePiece(c, j)){
               // Valoro negativamente que la ficha esté en casilla segura o meta.
               puntuacion_oponente++;
            }
            else if (estado.getBoard().getPiece(c, j).get_box().type == goal){
               puntuacion_oponente += 5;
            }
         }
      }

      // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
      return puntuacion_jugador - puntuacion_oponente;
   }
}

float Heuristica1::getHeuristic(const Parchis &estado, int jugador) const
{
   const float PEN_BARR = 0.3;
   const int RANGO_BARR = 25;

   float puntuacion_jugador = 0;
   float puntuacion_oponente = 0;

   int ganador = estado.getWinner();
   int oponente = (jugador + 1) % 2;

   // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
   if (ganador == jugador){
      return gana;
   }
   else if (ganador == oponente){
      return pierde;
   }
   else{
      // Colores que juega mi jugador y colores del oponente
      vector<color> my_colors = estado.getPlayerColors(jugador);
      vector<color> op_colors = estado.getPlayerColors(oponente);

      //.......................................
      //Vemos si estamos en la fase final de la partida

      bool faseFinal = ( (estado.piecesAtGoal(my_colors[0]) + estado.piecesAtGoal(my_colors[1])) == 3 ||
                         (estado.piecesAtGoal(op_colors[0]) + estado.piecesAtGoal(op_colors[1])) == 3  );

      float bonus_faseFinal = faseFinal ? 1 : 1;

      //.......................................
      // Recorro todas las fichas de mi jugador
      
      // Recorro colores de mi jugador.
      for (int i = 0; i < my_colors.size(); i++){

         color c = my_colors[i];

         // Recorro las fichas de ese color.
         for (int j = 0; j < num_pieces; j++){

            //1 Valoro positivamente que la ficha esté en casilla segura.
            if (estado.isSafePiece(c, j)){
               puntuacion_jugador += 1/bonus_faseFinal; 
            }
            //2 Valoro positivo si esta en casilla meta
            else if (estado.getBoard().getPiece(c, j).get_box().type == goal){
               puntuacion_jugador += 5*bonus_faseFinal ;
            }

            // //3 Valoro negativo si esta en casa
            // if (estado.getBoard().getPiece(c, j).get_box().type == home)
            //    puntuacion_jugador -= 3*bonus_faseFinal;
            
            // //Valoro negativo que haya rebote
            // if(estado.goalBounce()) 
            //    puntuacion_jugador -= 5;

            //3 Valoro mas negativo cuanto mas lejos de la meta esta
            puntuacion_jugador -= estado.distanceToGoal(c,j)*bonus_faseFinal;

            //5 Amenaza de ser comido y gestion de barreras
            Box posicion = estado.getBoard().getPiece(c,j).get_box();
            posicion.num = (((posicion.num - 30) % 68 + 68 ) % 68 );

            vector<pair<color,int>> fichas = estado.allPiecesBetween(posicion, estado.getBoard().getPiece(c,j).get_box() );
            
            for( int k=0; k<fichas.size(); k++ ){
               //si hay fichas del oponenete en las 14 casillas de detrás
               if( fichas[k].first == op_colors[0] || fichas[k].first == op_colors[1] ){
                  puntuacion_jugador-= 3/bonus_faseFinal;
                  
                  // //Gestion de Barreras
                  // if( estado.isWall( estado.getBoard().getPiece(c,j).get_box() ) ){
                  //    puntuacion_jugador += 1*bonus_faseFinal;

                  //    if( estado.distanceToGoal(fichas[k].first, fichas[k].second) < 20 ) //añado un plus si al rival le queda poca distancia a meta
                  //       puntuacion_jugador += 1*bonus_faseFinal;
                  // }
               }
            }

            // //Amenaza 2
            // vector<pair<color,int>> fichas = estado.allPiecesBetween(estado.getBoard().getPiece(c,j).get_box(), estado.getFinalBox(c) );
            // for( int k=0; k<fichas.size(); k++ ){
            //    if( fichas[k].first == op_colors[0] || fichas[k].first == op_colors[1] ){
            //       puntuacion_jugador+= 1/bonus_faseFinal;   //valoro positivamente que haya jugadores rivales delante
            //    }
            // }

            
            //Gestion de Barreras 2
            if( estado.getBoard().getPiece(c,j).get_box().type != final_queue ){

               vector<color> barreras = estado.anyWall(estado.getBoard().getPiece(c,j).get_box(), estado.getFinalBox(c) );

               for( int k=0; k<barreras.size(); k++ ){

                  int d = min(estado.distanceBoxtoBox(c, j, barreras[k], 0), RANGO_BARR);

                  if( barreras[k]==op_colors[0] || barreras[k]==op_colors[1] )
                     puntuacion_jugador -= PEN_BARR  * (RANGO_BARR-d);

                  if( barreras[k]==my_colors[0] || barreras[k]==my_colors[1] )
                     puntuacion_jugador -= 0.15  * (RANGO_BARR-d);
               }
            }
            
         }

         //6 Valoro negativo cuantas mas piezas haya en casa
         puntuacion_jugador -= estado.piecesAtHome(c)*2 * bonus_faseFinal;
      }

      //.......................................
      // Recorro todas las fichas del oponente
      float puntuacion_oponente = 0;

      // Recorro colores del oponente.
      for (int i = 0; i < op_colors.size(); i++){

         color c = op_colors[i];

         // Recorro las fichas de ese color.
         for (int j = 0; j < num_pieces; j++){

            //1 Valoro negativamente que la ficha esté en casilla segura o meta.
            if (estado.isSafePiece(c, j)){
               puntuacion_oponente += 1/bonus_faseFinal;
            }
            //2 Valoro negativo si esta en casilla meta
            else if (estado.getBoard().getPiece(c, j).get_box().type == goal){
               puntuacion_oponente += 5*bonus_faseFinal;
            }
            // //3 Valoro positivo si esta en casa
            // if (estado.getBoard().getPiece(c, j).get_box().type == home)
            //    puntuacion_oponente -= 3*bonus_faseFinal;
            
            // // Valoro positivo que haya rebote
            // if(estado.goalBounce()) 
            //    puntuacion_oponente -= 5;


            //3 Valoro positivo cuanto mas lejos de la meta esta
            puntuacion_oponente -= estado.distanceToGoal(c,j)*bonus_faseFinal;

            //5 Amenaza de ser comido y gestion de barreras
            Box posicion = estado.getBoard().getPiece(c,j).get_box();
            posicion.num = (((posicion.num - 30) % 68 + 68 ) % 68 );

            vector<pair<color,int>> fichas = estado.allPiecesBetween(posicion, estado.getBoard().getPiece(c,j).get_box() );
            
            for( int k=0; k<fichas.size(); k++ ){
               //si hay fichas del oponenete en las 14 casillas de detrás
               if( fichas[k].first == my_colors[0] || fichas[k].first == my_colors[1] ){
                  puntuacion_oponente-= 3/bonus_faseFinal;
                  
                  // //Gestion de Barreras
                  // if( estado.isWall( estado.getBoard().getPiece(c,j).get_box() ) ){
                  //    puntuacion_oponente += 1*bonus_faseFinal;

                  //    if( estado.distanceToGoal(fichas[k].first, fichas[k].second) < 20 ) //añado un plus si al rival le queda poca distancia a meta
                  //       puntuacion_oponente += 1*bonus_faseFinal;
                  // }
               }
            }

            // //Amenaza 2
            // vector<pair<color,int>> fichas = estado.allPiecesBetween(estado.getBoard().getPiece(c,j).get_box(), estado.getFinalBox(c) );
            // for( int k=0; k<fichas.size(); k++ ){
            //    if( fichas[k].first == my_colors[0] || fichas[k].first == my_colors[1] ){
            //       puntuacion_oponente+= 1/bonus_faseFinal;  //valoro negativamente que haya jugadores rivales delante
            //    }
            // }

            //Gestion de Barreras 2
            if( estado.getBoard().getPiece(c,j).get_box().type != final_queue ){

               const int caminillo_final = 8;
               Box ini_caminillo = estado.getBoard().getPiece(c,j).get_box();
               ini_caminillo.num = (ini_caminillo.num + (estado.distanceToGoal(c,j)-caminillo_final)) % 68;

               vector<color> barreras = estado.anyWall(estado.getBoard().getPiece(c,j).get_box(), estado.getFinalBox(c) );

               for( int k=0; k<barreras.size(); k++ ){

                  int d = min(estado.distanceBoxtoBox(c, j, barreras[k], 0), RANGO_BARR);

                  if( barreras[k]==my_colors[0] || barreras[k]==my_colors[1] )
                     puntuacion_oponente -= PEN_BARR  * (RANGO_BARR-d);

                  if( barreras[k]==op_colors[0] || barreras[k]==op_colors[1] )
                     puntuacion_jugador -= 0.15  * (RANGO_BARR-d);
               }            
            }


         }
         
         //6 Valoro positivo cuantas mas piezas haya en casa
         puntuacion_oponente -= estado.piecesAtHome(c)*2 * bonus_faseFinal;
      }

      //.......................................
      //5 Comida de ficha
      if( estado.isEatingMove() ){

         pair<color,int> eatenPiece = estado.eatenPiece();

         //comida a mi jugador
         if (eatenPiece.first == my_colors[0] || eatenPiece.first == my_colors[1]){

               // Si se ha comido el jugador a él mismo   //posible mejora: que sume puntuacion cuando sea ficha comida cerca de la casilla de salida
               if (estado.getCurrentPlayerId() == jugador){

                  // int distanciaMeta = estado.distanceToGoal(eatenPiece.first, eatenPiece.second);

                  // float factor = float(72 - distanciaMeta) / 72;
                  // factor = clamp(factor, 0.0f, 1.0f);
                  
                  puntuacion_jugador += (-5); //+ 20*factor);
               }
               // Si le ha comido el oponente
               else
                  puntuacion_jugador -= 30*bonus_faseFinal;
         }
         // Si la ficha comida es del oponente
         else{

               // Si se ha comido el oponente a sí mismo
               if (estado.getCurrentPlayerId() == oponente){
                  int distanciaMeta = estado.distanceToGoal(eatenPiece.first, eatenPiece.second);

                  float factor = float(72 - distanciaMeta) / 72;
                  factor = clamp(factor, 0.0f, 1.0f);

                  puntuacion_oponente += ((-5) + 20*factor);  
               }
               // Si le ha comido el jugador
               else
                  puntuacion_oponente -= 30*bonus_faseFinal;
         }
      }


      // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
      return puntuacion_jugador - puntuacion_oponente;
   }
}

float AIPlayer::Poda_AlfaBeta(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, float alpha, float beta, Heuristic *heuristic) const
{

   if (profundidad == profundidad_max || actual.gameOver())
   { // Nodo terminal o profundidad límite: llamo a la función heurística
      // IMPORTANTE: USAMOS EL MÉTODO evaluate AUNQUE HAYAMOS REDEFINIDO LA CLASE HEURISTIC
      return heuristic->evaluate(actual, jugador);
   }
   else if (actual.getCurrentPlayerId() == jugador)   // Nodo MAX
   { 
      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();
      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;

         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            if(profundidad == 0)
            {
               c_piece = it.getMovedColor();
               id_piece = it.getMovedPieceId();
               dice = it.getMovedDiceValue();
            }
            return (alpha);
         }

         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Poda_AlfaBeta(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, alpha, beta, heuristic);

         if (new_val > alpha)
         {
            // Me voy quedando con el máximo
            alpha = new_val;
            if (profundidad == 0)
            {
               // Almaceno el movimiento que me ha llevado al mejor valor (solo en la raíz)
               c_piece = it.getMovedColor();
               id_piece = it.getMovedPieceId();dice = it.getMovedDiceValue();
               dice = it.getMovedDiceValue();
            }
         }

         if( alpha>=beta ) break;  //podamos esa rama
      }

      return (alpha);
   }
   else   // Nodo MIN
   { 
      // Obtengo los hijos del nodo actual y los recorro
      ParchisBros rama = actual.getChildren();
      for (ParchisBros::Iterator it = rama.begin(); it != rama.end(); ++it)
      {
         Parchis nuevo_hijo = *it;
         
         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            return heuristic->evaluate(actual, jugador);
         }
         
         // Búsqueda en profundidad (llamada recursiva)
         float new_val = Poda_AlfaBeta(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, alpha, beta, heuristic);
         
         // Me voy quedando con el mínimo
         if (new_val < beta)
         {
            beta = new_val;
         }

         if( alpha>=beta ) break;  //podamos esa rama
      }

      return (beta);
   }
}

float AIPlayer::ProfundidadDinamica(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, float alpha, float beta, Heuristic *heuristic) const
{

   if (profundidad == profundidad_max || actual.gameOver())
   { // Nodo terminal o profundidad límite: llamo a la función heurística
      // IMPORTANTE: USAMOS EL MÉTODO evaluate AUNQUE HAYAMOS REDEFINIDO LA CLASE HEURISTIC
      return heuristic->evaluate(actual, jugador);
   }

   // Obtengo los hijos del nodo actual
   vector<ParchisSis> rama = actual.getChildrenList();

   //ajustamos profundidad en funcion de la ramificacion
   int ramificacion = rama.size();

   if (rama.empty()) return heuristic->evaluate(actual, jugador);
   
   int prof_temp = profundidad +1;
   int extra = (ramificacion <= 4)  ?  2 :
               (ramificacion <= 8)  ?  1 :
               (ramificacion >= 20) ? -2 :
               (ramificacion >= 12) ? -1 : 0;

   
   int prof_hijos = clamp(prof_temp + extra, prof_temp, 30);   //nunca se salga de los limites de profundidad minim y maxima

   
   if (actual.getCurrentPlayerId() == jugador)   // Nodo MAX
   { 
      for(int i = 0; i < rama.size(); i++){
         
         ParchisSis hijo_i = rama[i];     // Acceso al estado con el hijo i-ésimo del vector
         Parchis nuevo_hijo = *hijo_i;    // Acceso al estado

         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            if(profundidad == 0)
            {
               c_piece = hijo_i.getMovedColor();
               id_piece = hijo_i.getMovedPieceId();
               dice = hijo_i.getMovedDiceValue();
            }
            return (alpha);
         }

         // Búsqueda en profundidad (llamada recursiva)
         float new_val = ProfundidadDinamica(nuevo_hijo, jugador, profundidad + 1, prof_hijos, c_piece, id_piece, dice, alpha, beta, heuristic);

         if (new_val > alpha)
         {
            // Me voy quedando con el máximo
            alpha = new_val;
            if (profundidad == 0)
            {
               // Almaceno el movimiento que me ha llevado al mejor valor (solo en la raíz)
               c_piece = hijo_i.getMovedColor();
               id_piece = hijo_i.getMovedPieceId();
               dice = hijo_i.getMovedDiceValue();
            }
         }

         if( alpha>=beta ) break;  //podamos esa rama
      }

      return (alpha);
   }
   else   // Nodo MIN
   { 
      for(int i = 0; i < rama.size(); i++){
         
         ParchisSis hijo_i = rama[i];     // Acceso al estado con el hijo i-ésimo del vector
         Parchis nuevo_hijo = *hijo_i;    // Acceso al estado
         
         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            return heuristic->evaluate(actual, jugador);
         }
         
         // Búsqueda en profundidad (llamada recursiva)
         float new_val = ProfundidadDinamica(nuevo_hijo, jugador, profundidad + 1, prof_hijos, c_piece, id_piece, dice, alpha, beta, heuristic);
         
         // Me voy quedando con el mínimo
         if (new_val < beta)
         {
            beta = new_val;
         }

         if( alpha>=beta ) break;  //podamos esa rama
      }

      return (beta);
   }
}

float AIPlayer::OrdenacionMovimientos(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, float alpha, float beta, Heuristic *heuristic) const
{

   if (profundidad == profundidad_max || actual.gameOver())
   { // Nodo terminal o profundidad límite: llamo a la función heurística
      // IMPORTANTE: USAMOS EL MÉTODO evaluate AUNQUE HAYAMOS REDEFINIDO LA CLASE HEURISTIC
      return heuristic->evaluate(actual, jugador);
   }

   // Obtengo los hijos del nodo actual
   vector<ParchisSis> rama = actual.getChildrenList();

   //ajustamos profundidad en funcion de la ramificacion
   int ramificacion = rama.size();

   //determinar si vamos a hacer ordenacion o no en funcion de la profundidad y de la ramificacion
   bool ordenar = (profundidad <= 1) || (profundidad <= 3 && ramificacion <= 8); 

   //ordenamos
   if( ordenar ){

      bool esMax = (actual.getCurrentPlayerId() == jugador);

      sort(rama.begin(), rama.end(),[&](const ParchisSis &a, const ParchisSis &b)
      {
         const Parchis &pa = *a;                  // estado del primer hijo
         const Parchis &pb = *b;                  // estado del segundo hijo

         float va = heuristic->evaluate(pa, jugador);
         float vb = heuristic->evaluate(pb, jugador);

         // MAX: de mayor a menor   |   MIN: de menor a mayor
         return (esMax ? (va > vb)  :  (va < vb));
      });
   }


   if (actual.getCurrentPlayerId() == jugador)   // Nodo MAX
   { 

      for(int i = 0; i < rama.size(); i++){
         
         ParchisSis hijo_i = rama[i];     // Acceso al estado con el hijo i-ésimo del vector
         Parchis nuevo_hijo = *hijo_i;    // Acceso al estado

         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            if(profundidad == 0)
            {
               c_piece = hijo_i.getMovedColor();
               id_piece = hijo_i.getMovedPieceId();
               dice = hijo_i.getMovedDiceValue();
            }
            return (alpha);
         }

         // Búsqueda en profundidad (llamada recursiva)
         float new_val = OrdenacionMovimientos(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, alpha, beta, heuristic);

         if (new_val > alpha)
         {
            // Me voy quedando con el máximo
            alpha = new_val;
            if (profundidad == 0)
            {
               // Almaceno el movimiento que me ha llevado al mejor valor (solo en la raíz)
               c_piece = hijo_i.getMovedColor();
               id_piece = hijo_i.getMovedPieceId();
               dice = hijo_i.getMovedDiceValue();
            }
         }

         if( alpha>=beta ) break;  //podamos esa rama
      }

      return (alpha);
   }
   else   // Nodo MIN
   { 
      for(int i = 0; i < rama.size(); i++){
         
         ParchisSis hijo_i = rama[i];     // Acceso al estado con el hijo i-ésimo del vector
         Parchis nuevo_hijo = *hijo_i;    // Acceso al estado
         
         // Verificar si hemos alcanzado el límite
         if (NodeCounter::isLimitReached())
         {
            cout << "Límite de nodos alcanzado, devolviendo el mejor nodo parcial" << endl;
            return heuristic->evaluate(actual, jugador);
         }
         
         // Búsqueda en profundidad (llamada recursiva)
         float new_val = OrdenacionMovimientos(nuevo_hijo, jugador, profundidad + 1, profundidad_max, c_piece, id_piece, dice, alpha, beta, heuristic);
         
         // Me voy quedando con el mínimo
         if (new_val < beta)
         {
            beta = new_val;
         }

         if( alpha>=beta ) break;  //podamos esa rama
      }

      return (beta);
   }
}
