package por;

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
public class Practicum {

    private static Logger logger = Logger.getLogger(Practicum.class);

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        
        PORPropertyConfigurator.configureProperties();
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
        logger.info("Entering the class");

        // TODO code application logic here
        new CloudSelector().setVisible(true);
        // KeyStore.storeKey("acd", "file1", "jied", "razzyfresh");

        logger.info("Exiting the class");

    }
}
