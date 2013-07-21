/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package por;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 *
 * @author poojad
 */
public class PORPropertyConfigurator {

    private static Logger logger = Logger.getLogger(PORPropertyConfigurator.class);
    static Properties prop = new Properties();
    protected static String logger_path = null;
    protected static String ami_id = null;
    protected static String instance_user_name = null;
    protected static String executable_path = null;
    protected static String file_to_be_mounted = null;
    protected static String mount_path = null;
    protected static String default_partition_size = null;
    protected static String truecrypt_password = null;
    protected static String random_file = null;
    protected static String key_file = null;
    protected static String keystore_file_name = null;
    protected static String chart_path = null;
    protected static String por_encoder_executable = null;
    protected static String por_decoder_executable = null;
    protected static String por_server_executable = null;
    protected static String por_client_executable = null;

    public static void configureProperties() {
        try {

             //load a properties file from class path, inside static method
    	   //prop.load(App.class.getClassLoader().getResourceAsStream("config.properties");));
 
            //load a properties file
            prop.load(new FileInputStream("/home/poojad/POR/config/Config.properties"));

            //get the property value and print it out
            if (prop.getProperty("LOGGER_PATH") != null) {
                logger_path = prop.getProperty("LOGGER_PATH").trim();
            } else {
                logger.info("cannot find property logger_path");
                throw new RuntimeException("cannot find property logger_path");
            }

            PropertyConfigurator.configure(logger_path);

            //get the property value and print it out
            if (prop.getProperty("AMI_ID") != null) {
                ami_id = prop.getProperty("AMI_ID").trim();
            } else {
                logger.info("cannot find property AMI_ID");
                throw new RuntimeException("cannot find property AMI_ID");
            }

            //get the property value and print it out
            if (prop.getProperty("INSTANCE_USER_NAME") != null) {
                instance_user_name = prop.getProperty("INSTANCE_USER_NAME").trim();
            } else {
                logger.info("cannot find property INSTANCE_USER_NAME");
                throw new RuntimeException("cannot find property INSTANCE_USER_NAME");
            }

            if (prop.getProperty("INSTANCE_USER_NAME") != null) {
                instance_user_name = prop.getProperty("INSTANCE_USER_NAME").trim();
            } else {
                logger.info("cannot find property INSTANCE_USER_NAME");
                throw new RuntimeException("cannot find property INSTANCE_USER_NAME");
            }

            if (prop.getProperty("EXECUTABLE_PATH") != null) {
                executable_path = prop.getProperty("EXECUTABLE_PATH").trim();
            } else {
                logger.info("cannot find property EXECUTABLE_PATH");
                throw new RuntimeException("cannot find property EXECUTABLE_PATH");
            }

            if (prop.getProperty("FILE_TO_BE_MOUNTED") != null) {
                file_to_be_mounted = prop.getProperty("FILE_TO_BE_MOUNTED").trim();
            } else {
                logger.info("cannot find property FILE_TO_BE_MOUNTED");
                throw new RuntimeException("cannot find property FILE_TO_BE_MOUNTED");
            }

            if (prop.getProperty("MOUNT_PATH") != null) {
                mount_path = prop.getProperty("MOUNT_PATH").trim();
            } else {
                logger.info("cannot find property MOUNT_PATH");
                throw new RuntimeException("cannot find property MOUNT_PATH");
            }

            if (prop.getProperty("DEFAULT_PARTITION_SIZE") != null) {
                default_partition_size = prop.getProperty("DEFAULT_PARTITION_SIZE").trim();
            } else {
                logger.info("cannot find property DEFAULT_PARTITION_SIZE");
                throw new RuntimeException("cannot find property DEFAULT_PARTITION_SIZE");
            }


            if (prop.getProperty("TRUECRYPT_PASSWORD") != null) {
                truecrypt_password = prop.getProperty("TRUECRYPT_PASSWORD").trim();
            } else {
                logger.info("cannot find property TRUECRYPT_PASSWORD");
                throw new RuntimeException("cannot find property TRUECRYPT_PASSWORD");
            }

            if (prop.getProperty("RANDOM_FILE") != null) {
                random_file = prop.getProperty("RANDOM_FILE").trim();
            } else {
                logger.info("cannot find property RANDOM_FILE");
                throw new RuntimeException("cannot find property RANDOM_FILE");
            }

            if (prop.getProperty("KEY_FILE") != null) {
                key_file = prop.getProperty("KEY_FILE").trim();
            } else {
                logger.info("cannot find property KEY_FILE");
                throw new RuntimeException("cannot find property KEY_FILE");
            }

            if (prop.getProperty("KEYSTORE_FILE_NAME") != null) {
                keystore_file_name = prop.getProperty("KEYSTORE_FILE_NAME").trim();
            } else {
                logger.info("cannot find property KEYSTORE_FILE_NAME");
                throw new RuntimeException("cannot find property KEYSTORE_FILE_NAME");
            }

            if (prop.getProperty("CHART_PATH") != null) {
                chart_path = prop.getProperty("CHART_PATH").trim();
            } else {
                logger.info("cannot find property CHART_PATH");
                throw new RuntimeException("cannot find property CHART_PATH");
            }

            if (prop.getProperty("POR_ENCODER_EXECUTABLE") != null) {
                por_encoder_executable = prop.getProperty("POR_ENCODER_EXECUTABLE").trim();
            } else {
                logger.info("cannot find property POR_ENCODER_EXECUTABLE");
                throw new RuntimeException("cannot find property POR_ENCODER_EXECUTABLE");
            }

            if (prop.getProperty("POR_DECODER_EXECUTABLE") != null) {
                por_decoder_executable = prop.getProperty("POR_DECODER_EXECUTABLE").trim();
            } else {
                logger.info("cannot find property POR_DECODER_EXECUTABLE");
                throw new RuntimeException("cannot find property POR_DECODER_EXECUTABLE");
            }

            if (prop.getProperty("POR_SERVER_EXECUTABLE") != null) {
                por_server_executable = prop.getProperty("POR_SERVER_EXECUTABLE").trim();
            } else {
                logger.info("cannot find property POR_SERVER_EXECUTABLE");
                throw new RuntimeException("cannot find property POR_SERVER_EXECUTABLE");
            }

            if (prop.getProperty("POR_CLIENT_EXECUTABLE") != null) {
                por_client_executable = prop.getProperty("POR_CLIENT_EXECUTABLE").trim();
            } else {
                logger.info("cannot find property POR_CLIENT_EXECUTABLE");
                throw new RuntimeException("cannot find property POR_CLIENT_EXECUTABLE");
            }

        } catch (IOException ex) {
            logger.info(ex.getMessage());
        }
    }
}
