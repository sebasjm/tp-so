#ifndef __tecladoConsola_h
#define __tecladoConsola_h

#include <fstream>
#include <string>
#include <sys/ipc.h>

#include <ipc/point.h>
#include <messages/teclado-consola.h>

using namespace std;

class TecladoConsolaClass : public PointClass<line> {
public:
    TecladoConsolaClass();
    TecladoConsolaClass(const int i);
    ~TecladoConsolaClass();

    PointClass<line>& operator<<(const line);
    PointClass<line>& operator>>(line&);

    bool isClosed();
private:
};

#endif
