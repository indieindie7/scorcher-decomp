// Finds calls into routine-table slots: CALL [reg+disp] or CALL reg where one of the previous 6 instructions loaded reg
// from [reg2+disp], disp in 0x10..0x30 (RoutPtr[0..8] of _ConRouts). Prints: addr function instruction (load).  args: <out.txt>
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.lang.Register;
import ghidra.program.model.scalar.Scalar;
import java.io.*;
public class FindSlotCalls extends GhidraScript {
    long disp(Instruction ins, int op) {
        boolean reg = false; long d = -1;
        for (Object o : ins.getOpObjects(op)) { if (o instanceof Register) reg = true; if (o instanceof Scalar) d = ((Scalar) o).getSignedValue(); }
        return reg && d >= 0x10 && d <= 0x30 && (d & 3) == 0 ? d : -1;
    }
    public void run() throws Exception {
        PrintWriter w = new PrintWriter(new FileWriter(getScriptArgs()[0]));
        InstructionIterator it = currentProgram.getListing().getInstructions(true);
        int n = 0;
        while (it.hasNext()) {
            Instruction ins = it.next();
            String m = ins.getMnemonicString();
            if (!(m.equals("CALL") || m.equals("JSR")) || ins.getNumOperands() != 1) continue;
            String why = null;
            if (disp(ins, 0) >= 0) why = "direct";
            else {
                Register r = ins.getRegister(0);
                if (r == null) continue;
                Instruction p = ins.getPrevious();
                for (int k = 0; k < 6 && p != null; k++, p = p.getPrevious()) {
                    if (p.getNumOperands() == 2 && p.getMnemonicString().startsWith("MOV") && r.equals(p.getRegister(0)) && disp(p, 1) >= 0) { why = p.getAddress() + " " + p; break; }
                    if (p.getNumOperands() >= 1 && r.equals(p.getRegister(0))) break;   // reg overwritten by something else
                }
            }
            if (why == null) continue;
            Function f = getFunctionContaining(ins.getAddress());
            w.println(ins.getAddress() + " " + (f == null ? "?" : f.getName()) + " " + ins + "   <- " + why);
            n++;
        }
        w.close(); println("FindSlotCalls: " + n + " hits");
    }
}
