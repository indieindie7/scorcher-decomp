// Prints the calling convention and decompiled signature of a few routine-table functions.
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
public class CheckConvention extends GhidraScript {
    @Override
    public void run() throws Exception {
        println("compiler spec: " + currentProgram.getCompilerSpec().getCompilerSpecID() + ", default proto: " + currentProgram.getCompilerSpec().getDefaultCallingConvention().getName());
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        int shown = 0;
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
            if (f.getParameterCount() < 2 || shown >= 8) continue;
            DecompileResults r = d.decompileFunction(f, 20, monitor);
            String sig = r.decompileCompleted() ? r.getDecompiledFunction().getSignature() : "(decompile failed)";
            println(f.getName() + " conv=" + f.getCallingConventionName() + " :: " + sig);
            shown++;
        }
        d.dispose();
    }
}
