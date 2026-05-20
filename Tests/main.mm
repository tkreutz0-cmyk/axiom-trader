// Tests/main.mm
#include "Axiom/Trade.hpp"
#include "src/core/utils/fixed_point.hpp"
#include <cassert>
#include <iostream>

int main() {
    Axiom::Trade t;
    t.setSymbol("EUR/USD");
    
    // ✅ PRIO 4: Initialisierung über das neue fixed_point System (Skalierung 1:10000)
    t.entry = core::utils::Money::from_raw(11000); // 1.1000
    t.exit  = core::utils::Money::from_raw(11010); // 1.1010
    t.units = core::utils::Money::from_raw(10000); // 1.0 Units
    
    // FX Erkennung prüfen
    assert(t.meta.isFX == true);
    assert(t.meta.isJPY == false);
    
    // ✅ PRIO 4: Exakter, deterministischer Pip-Vergleich ohne Fließkomma-Toleranzen
    // 1.1010 - 1.1000 = 0.0010 * 10000 = 10 Pips (Interner Rohwert für 10 Pips = 100000)
    core::utils::Money calculatedPips = t.calculatePips();
    assert(calculatedPips.raw() == 100000); // 10.0000 Pips im Festkomma-Format
    
    // ✅ PRIO 4: PnL-Berechnung validieren (10 Pips * 10 USD/pip * 1 Lot = 100 USD)
    // Interner Rohwert für 100 USD = 1000000
    core::utils::Money pipValue = core::utils::Money(10); // 10.0000 USD
    core::utils::Money pnl = t.calculatePnL(pipValue, true);
    assert(pnl.raw() == 1000000); // 100.0000 USD exakt
    
    std::cout << "🚀 [Axiom Test Suite]: Alle deterministischen Invarianten erfolgreich validiert! Build STABLE." << std::endl;
    return 0;
}

