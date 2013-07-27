package por;

/**
 *
 * @author rucha
 */

import por.util.PORPropertyConfigurator;
import java.io.IOException;
import java.util.*;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public abstract class CloudProvider extends javax.swing.JFrame {

    public static int providerId = 1; // for Amazon cloud provider
    static Logger logger = Logger.getLogger(MainMenu.class);

    public void stopInstanceGeneric() {
        if (providerId == 1) {
            try {
                // calls the method to stop instance, specific to Amazon cloud provider
                ConnectToAmazonEC2.stopInstance();
                // disposes the JFrame
                dispose();
                logger.info("Exiting the class");
                // exits
                System.exit(0);
            } catch (InterruptedException ex) {
                ex.printStackTrace();
                System.exit(1);
            }
        }

    }
}
