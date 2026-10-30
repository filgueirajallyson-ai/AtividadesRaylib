#ifndef INVENTARIO_H
#define INVENTARIO_H

#include "entidade.h"

typedef struct inventario Inventario;

Inventario *inventarioCriar(int capacidade);
void        inventarioDestruir(Inventario *inv);

bool        inventarioAdicionar(Inventario *inv, Entidade *e);
void        inventarioRemover(Inventario *inv, int indice);

Entidade   *inventarioObter(Inventario *inv, int indice);
int         inventarioTotal(Inventario *inv);

bool        inventarioEstaCheio(Inventario *inv);
int         inventarioContarPorTipo(Inventario *inv, TipoEntidade tipo);

#endif
