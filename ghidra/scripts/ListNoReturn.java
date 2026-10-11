// Print every function flagged non-returning, with its size and call-site count.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class ListNoReturn extends GhidraScript {
    public void run() throws Exception {
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
            if (!f.hasNoReturn()) continue;
            int n = 0;
            for (Reference r : currentProgram.getReferenceManager().getReferencesTo(f.getEntryPoint())) if (r.getReferenceType().isCall()) n++;
            println("NORETURN " + f.getEntryPoint() + " " + f.getName() + " size=" + f.getBody().getNumAddresses() + " callers=" + n);
        }
    }
}
