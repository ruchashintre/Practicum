package edu.cmu.por;

import edu.cmu.por.util.PORPropertyConfigurator;
import java.io.IOException;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author PoojaD
 */
public class Main {

    private static Logger logger = Logger.getLogger(Main.class);

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        try {
            PORPropertyConfigurator.configureProperties();
        } catch (IOException ioe) {
            System.out.println("Error in setting up the path of the workspace : " + ioe.getMessage());
        }
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
        logger.info("Entering the class");

        // TODO code application logic here
        new CloudSelector().setVisible(true);
        // KeyStore.storeKey("acd", "file1", "jied", "razzyfresh");

        logger.info("Exiting the class");

    }
}
