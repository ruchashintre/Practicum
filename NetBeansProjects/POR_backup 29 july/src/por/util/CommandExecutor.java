/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package por.util;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 *
 * @author poojad
 */
public class CommandExecutor {

    static Logger logger = Logger.getLogger(CommandExecutor.class);
    public static Process prcs = null;
  

    CommandExecutor() {
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
    }

    public static void executeCommandAndExit(String command) throws IOException, InterruptedException {

        logger.info("Entering method executeCommand");
        Runtime rt = Runtime.getRuntime();
        String line = null;

        logger.info("Command to be executed = " + command);
        prcs = rt.exec(command);

        BufferedReader stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

        logger.info("ERROR in the process - (if any)");
        while ((line = stdError.readLine()) != null) {
            logger.info(line);
        }
        logger.info("End of error");


        BufferedReader stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
        logger.info("Output of the process - (if any)");

        while ((line = stdInput.readLine()) != null) {
            logger.info(line);
        }
        logger.info("End of output");

        int exitVal = prcs.waitFor();
        logger.info("Process exitValue: " + exitVal);

        prcs.destroy();

        logger.info("Exiting the method");
    }

    
   

    
    public static void executeInTerminalCommandAndExit(String command) throws IOException, InterruptedException {

        String line = null;
        logger.info("Entering method executeCommand");
        PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);
        logger.info("Command to be executed = " + command);
        out.println(command);
        out.println("exit");

        BufferedReader stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

        logger.info("ERROR in the process - (if any)");
        while ((line = stdError.readLine()) != null) {
            logger.info(line);
        }
        logger.info("End of error");

        BufferedReader stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
        logger.info("Output of the process - (if any)");
        while ((line = stdInput.readLine()) != null) {
            logger.info(line);
        }
        logger.info("End of output");
        int exitVal = prcs.waitFor();
        logger.info("Process exitValue: " + exitVal);

        prcs.destroy();
    }

    public static void executeInTerminalCommand(String command) throws IOException {

        String line = null;
        logger.info("Entering method executeCommand");
        PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);
        logger.info("Command to be executed = " + command);
        out.println(command);

        logger.info("End of output");
    }
}
