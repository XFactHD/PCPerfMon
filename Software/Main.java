import java.io.*;

public class Main
{
    public static void main(String[] args) throws IOException, InterruptedException
    {
        BufferedReader sysIn = new BufferedReader(new InputStreamReader(System.in));

        Process p = new ProcessBuilder("ohm_reader.exe")
                .redirectError(ProcessBuilder.Redirect.INHERIT)
                .start();

        BufferedReader pIn = new BufferedReader(new InputStreamReader(p.getInputStream()));
        BufferedWriter pOut = new BufferedWriter(new OutputStreamWriter(p.getOutputStream()));

        //Relay start info
        String lineStart = pIn.readLine();
        System.out.println(lineStart);

        while (true)
        {
            String lineIn = sysIn.readLine();
            //System.err.println("[JAVA] Command received");
            pOut.write(lineIn + "\r\n");
            pOut.flush();

            if (lineIn.startsWith("shutdown"))
            {
                //System.err.println("[JAVA] Received shutdown");
                p.waitFor();
                //System.err.println("[JAVA] Process exited with " + p.exitValue());
                break;
            }
            else
            {
                String lineOut = pIn.readLine();
                //System.err.println("[JAVA] Reply received");
                System.out.println(lineOut);
            }
        }
    }
}
