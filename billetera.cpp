#include <vector>

#include "lib.h"
#include "calendario.h"
#include "billetera.h"
#include "blockchain.h"

using namespace std;

Billetera::Billetera(const id_billetera id, Blockchain* blockchain)
  : _id(id)
  , _blockchain(blockchain)
  , _saldo(0) {
}

id_billetera Billetera::id() const {
  return _id;
}

void Billetera::notificar_transaccion(Transaccion t) {
  //chequeamos que la transaccion sea valida
  bool billeteras_distintas = t.origen != t.destino;

  //si es la billetera origen
  if(t.origen == _id && t.origen != t.destino){
    //1. se resta el monto
    _saldo = _saldo - t.monto;
    //2.Si la clave t.destino ya estaba en el map se le suma uno,
    //si no, se crea y se le suma uno al cero que pone por default.
    _billeteras_frecuentes[t.destino] ++;
  } 
  //Si la billetera es destino
  if (t.destino == _id){
    //Se suma el monto
    _saldo = _saldo + t.monto;
  }
  
  //Agrego la transaccion al historial de la billetera
  bool transaccion_relevante = t.origen == _id || t.destino == _id;
  if(transaccion_relevante){
    _historial.push_back(t);
  
    //agrego el saldo del final del dia a un map.
    timestamp fin_dia = Calendario::fin_del_dia(t._timestamp);
    _saldo_x_dia[fin_dia] = _saldo;
  }
}

monto Billetera::saldo() const {
  return _saldo;
}

monto Billetera::saldo_al_fin_del_dia(timestamp t) const {
  timestamp fin_dia = Calendario::fin_del_dia(t);
  
  auto it = _saldo_x_dia.upper_bound(fin_dia);
  //Si no hay saldo registrado para ese día (antes de la creación)
  if (it == _saldo_x_dia.begin()) {
    return 0;
  }

  --it;
  return it->second;
}

vector<Transaccion> Billetera::ultimas_transacciones(int k) const {
  //creo un vector para pushearle las k transacciones
  vector<Transaccion> ret;
  //el iterador comienza desde el final del historial
  auto it = _historial.rbegin();
  while(it != _historial.rend() && ret.size() < k){
    //pusheo la transaccion al vector
    ret.push_back(*it);
    ++it;
  }

  return ret;
}

vector<id_billetera> Billetera::detinatarios_mas_frecuentes(int k) const {
  const list<Transaccion> transacciones = _blockchain->transacciones();

  // cuento la cantidad de transacciones salientes por cada billetera de destino
  map<id_billetera, int> transacciones_por_billetera;
  auto it = transacciones.begin();
  while (it != transacciones.end()) {
    if (it->origen == _id) {
      // notar que el map devuelve 0 por default!
      transacciones_por_billetera[it->destino]++;
    }
    ++it;
  }

  // invierto el map de forma que puedo accedes a las billeteras según su
  // cantidad de transacciones.
  map<int, vector<id_billetera>> billeteras_por_cantidad_de_transacciones;
  auto it2 = transacciones_por_billetera.begin();
  while (it2 != transacciones_por_billetera.end()) {
    billeteras_por_cantidad_de_transacciones[it2->second].push_back(it2->first);
    ++it2;
  }

  // recorro el map usando un iterador en orden inverso, que me lleva por todos
  // los pares de entradas desde la mayor clave hasta la menor.
  vector<id_billetera> ret = {};
  auto it3 = billeteras_por_cantidad_de_transacciones.rbegin();
  while (it3 != billeteras_por_cantidad_de_transacciones.rend() && ret.size() < k) {
    vector<id_billetera> siguiente_grupo = it3->second;
    int i = 0;
    while (i < siguiente_grupo.size() && ret.size() < k) {
      ret.push_back(siguiente_grupo[i]);
      i++;
    }
    ++it3;
  }

  return ret;
}
