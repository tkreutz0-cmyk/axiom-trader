#include "Axiom/Trade.hpp"
#include <cassert>

int main() {
    Axiom::Trade t;
    t.setSymbol("EUR/USD");
    t.entry = 1.1000;
    t.exit  = 1.1010;
    t.units = 1.0;

    // FX detection
    assert(t.meta.isFX == true);
    assert(t.meta.isJPY == false);

    // pips: (0.0010)*10000 = 10
    assert(t.calculatePips() > 9.999 && t.calculatePips() < 10.001);

    // money: 10 pips * 10 USD/pip/lot * 1 lot = 100 USD
    const double pnl = t.calculatePnL(10.0, true);
    assert(pnl > 99.9 && pnl < 100.1);

    return 0;
}
