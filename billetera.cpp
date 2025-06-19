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
  //invierto las claves y valores del map para poder recorrerlo por cant de envios
  map<int, id_billetera> ordenados;
  for (const auto& [dest, cant] : _billeteras_frecuentes) {
    ordenados[cant] = dest;
  }

  //creo un vector al que ponerle los destinatarios
  vector<id_billetera> ret;
  //como el map se recorre en orden ascendente, lo recorro a desde el final.
  for(auto it = ordenados.rbegin(); it != ordenados.rend() && ret.size()<k ; ++it){
    ret.push_back(it->second);
  }
  return ret;
}
