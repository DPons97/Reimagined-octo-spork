#include <stdio.h>
#include <iostream>

// Additional libraries and classes
#include "Logger.h"

int main() {
    auto NewLog = new Logger("LogBello");

    NewLog->WriteLog("Ciao, sono Michele. Chiama la MAMMAAAAAA!!!!! E scamma la NONNNAAAAAAAAAAS");

    free(NewLog);

    return 0;
}

