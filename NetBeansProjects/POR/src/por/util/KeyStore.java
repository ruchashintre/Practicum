package por.util;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import por.ConnectToAmazonCloud;
/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author PoojaD
 */
public abstract class KeyStore {

    private static Logger logger = Logger.getLogger(KeyStore.class);

    KeyStore() {
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
    }

    public static void storeKey(String userName, String fileName, String masterKey, String admPassword) throws IOException, InterruptedException {
        logger.info("Entering method storeKey");
        String line = null;
        File file = null;
        File keyFile = null;

        file = new File(PORPropertyConfigurator.file_to_be_mounted);

        //if file doesnt exists, then create it
        if (!file.exists()) {
            logger.info(PORPropertyConfigurator.file_to_be_mounted + "does not exists");
            file.createNewFile();

            //create the volume
            StringBuilder command = new StringBuilder("truecrypt -t --size=");
            command.append(PORPropertyConfigurator.default_partition_size);
            command.append(" --password=");
            command.append(PORPropertyConfigurator.truecrypt_password);
            command.append("  --random-source=");
            command.append(PORPropertyConfigurator.random_file);
            command.append(" -k ");
            command.append(PORPropertyConfigurator.key_file);
            command.append(" --volume-type=normal --encryption=AES --hash=SHA-512 --filesystem=FAT -c ");
            command.append(PORPropertyConfigurator.file_to_be_mounted);
            CommandExecutor.executeCommandAndExit(command.toString());

            try {
                mount(userName, fileName, masterKey, admPassword);
            } catch (InterruptedException ioe) {
                throw new RuntimeException("Unexpected error in mounting the volume");
            }

            logger.info("Volume mounted");

            keyFile = new File(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name);
            if (!keyFile.exists()) {
                keyFile.createNewFile();
                logger.info(PORPropertyConfigurator.keystore_file_name + " created");
            }

            FileWriter fileWritter = new FileWriter(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name, true);
            try (BufferedWriter bufferWritter = new BufferedWriter(fileWritter)) {
                StringBuffer str = new StringBuffer();
                str.append(userName);
                str.append("|");
                str.append(fileName);
                str.append("|");
                str.append(masterKey);
                str.append("\n");
                logger.info("Appended to the file" + str);
                bufferWritter.write(str.toString());
            }

        } else {

            //mount a volume
            logger.info("Volume is present.");

            try {
                mount(userName, fileName, masterKey, admPassword);
            } catch (InterruptedException ioe) {
                throw new RuntimeException("Unexpected error in mounting the volume");
            }

            logger.info("Volume is mounted now");

            FileWriter fileWritter = new FileWriter(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name, true);
            try (BufferedWriter bufferWritter = new BufferedWriter(fileWritter)) {
                StringBuffer str = new StringBuffer();
                str.append(userName);
                str.append("|");
                str.append(fileName);
                str.append("|");
                str.append(masterKey);
                str.append("\n");
                logger.info("Appended to the file" + str);
                bufferWritter.write(str.toString());
            }
        }

        try {
            dismount(userName, fileName, masterKey, admPassword);
        } catch (InterruptedException ie) {
            throw new RuntimeException("Unexpected error in dismounting the volume");
        }
        logger.info("Exiting the method");

    }

    public static void validateKey(String userName, String fileName, String masterKey, String admPassword) throws IOException {

        File file;
        String sCurrentLine = null;
        BufferedReader br = null;

        file = new File(PORPropertyConfigurator.file_to_be_mounted);

        if (!file.exists()) {
            logger.info("Mounted partition not found. Please contact administrator" + PORPropertyConfigurator.file_to_be_mounted);
            throw new RuntimeException("Mounted partition not found. Please contact administrator" + PORPropertyConfigurator.file_to_be_mounted);
        }

        try {
            mount(userName, fileName, masterKey, admPassword);
        } catch (InterruptedException ioe) {
            throw new RuntimeException("Unexpected error in mounting the volume");
        }
        logger.info("partition mounted");

        br = new BufferedReader(new FileReader(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name));

        boolean flag = false;
        while ((sCurrentLine = br.readLine()) != null) {

            if (sCurrentLine.startsWith(userName)) {
                String array[] = sCurrentLine.split("\\|");
                if (array[0].equalsIgnoreCase(userName)
                        && array[1].contains(fileName)
                        && array[2].equalsIgnoreCase(masterKey)) {
                    logger.info("filename and masterkey validated");
                    flag = true;
                }
            }
        }
        if (!flag) {
            throw new RuntimeException("Filename and master key do not match. Kindly re-enter");
        }

        try {
            dismount(userName, fileName, masterKey, admPassword);
        } catch (InterruptedException ie) {
            throw new RuntimeException("Unexpected error in dismounting the volume");
        }
        logger.info("Exiting the method");
    }

    public static void mount(String userName, String fileName, String masterKey, String admPassword) throws IOException, InterruptedException {
        logger.info("Entering method");

        String command = "truecrypt " + PORPropertyConfigurator.file_to_be_mounted 
                + " " + PORPropertyConfigurator.mount_path 
                + " -k=" + PORPropertyConfigurator.key_file 
                + " -p=" + PORPropertyConfigurator.truecrypt_password;
        //+ " --protect-hidden=no --password=" 
          //      + admPassword;
        CommandExecutor.executeCommandAndExit(command);

        logger.info("Exiting method");

    }

    public static void dismount(String userName, String fileName, String masterKey, String passString) throws IOException, InterruptedException {

        logger.info("Entering the method");
        String command = "truecrypt -d " ;
             //  + PORPropertyConfigurator.mount_path
               // + "--password="
               // + passString;
        CommandExecutor.executeCommandAndExit(command);

        logger.info("Exiting the process");
    }
}