// ReflowBody.java: args are <entry>[:<extra>,<extra>...] (hex). Disassembles by flow from the entry and from each extra
// address (flow targets Ghidra never decoded, e.g. FlowReach "noinst"), then recomputes the function body from its flow
// (CreateFunctionCmd.fixupFunctionBody). Keeps the name. For bodies cut short of code their own flow reaches.
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;

public class ReflowBody extends GhidraScript {
    @Override
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String arg : getScriptArgs()) {
            String[] p = arg.split(":");
            Address e = toAddr(Long.parseLong(p[0], 16));
            Function f = fm.getFunctionAt(e);
            if (f == null) { println("ReflowBody: no function at " + p[0]); continue; }
            new DisassembleCommand(e, null, true).applyTo(currentProgram, monitor);
            if (p.length > 1) for (String x : p[1].split(",")) new DisassembleCommand(toAddr(Long.parseLong(x, 16)), null, true).applyTo(currentProgram, monitor);
            long old = f.getBody().getNumAddresses();
            boolean ok = CreateFunctionCmd.fixupFunctionBody(currentProgram, f, monitor);
            println("ReflowBody: " + f.getName() + " " + old + " -> " + f.getBody().getNumAddresses() + " max=" + f.getBody().getMaxAddress() + (ok ? "" : " FAILED"));
        }
    }
}
