// Removes functions (and their disassembly) whose entry lies in a non-executable memory block.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import java.util.*;

public class CleanDataFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        Memory mem = currentProgram.getMemory();
        List<Function> bad = new ArrayList<>();
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
            MemoryBlock b = mem.getBlock(f.getEntryPoint());
            if (b != null && !b.isExecute()) bad.add(f);
        }
        for (Function f : bad) { removeFunction(f); clearListing(f.getBody()); }
        println("CleanDataFunctions: removed " + bad.size() + " functions in non-executable blocks, " + currentProgram.getFunctionManager().getFunctionCount() + " remain");
    }
}
