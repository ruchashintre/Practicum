package por;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
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
public abstract class KeyStore {

    static Logger logger = Logger.getLogger(KeyStore.class);

    KeyStore() {
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
    }

    public static int storeKey(String userName, String fileName, String masterKey, String admPassword) {
        logger.info("Entering method storeKey");
        String line = null;
        File file = null;
        File keyFile = null;

        file = new File(PORPropertyConfigurator.file_to_be_mounted);

        try {
            //if file doesnt exists, then create it
            if (!file.exists()) {
                logger.info(PORPropertyConfigurator.file_to_be_mounted + "does not exists");

                file.createNewFile();

                //create the volume
                String command = "truecrypt -t --size=" + PORPropertyConfigurator.default_partition_size + " --password=" + PORPropertyConfigurator.truecrypt_password + "  --random-source=" + PORPropertyConfigurator.random_file + " -k " + PORPropertyConfigurator.key_file + " --volume-type=normal --encryption=AES --hash=SHA-512 --filesystem=FAT " + "-c " + PORPropertyConfigurator.file_to_be_mounted;

                Process p = Runtime.getRuntime().exec(command);
                logger.info(command);
                BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
                logger.info("Output of the process - create Volume");

                while ((line = stdInput.readLine()) != null) {
                    logger.info(line);
                }
                logger.info("End of output");

                BufferedReader stdError = new BufferedReader(new InputStreamReader(p.getErrorStream()));

                logger.info("ERROR in the process - create volume(if any)");
                while ((line = stdError.readLine()) != null) {
                    logger.info(line);
                }
                logger.info("End of error");

                int exitVal = p.waitFor();
                logger.info("Process exitValue: " + exitVal);

                p.destroy();
                //mount a volume
                mount(userName, fileName, masterKey, admPassword);

                logger.info("Volume mounted");
                keyFile = new File(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name);
                if (!keyFile.exists()) {
                    keyFile.createNewFile();
                    logger.info(PORPropertyConfigurator.keystore_file_name + " created");
                }

                FileWriter fileWritter = new FileWriter(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name, true);
                try (BufferedWriter bufferWritter = new BufferedWriter(fileWritter)) {
                    String str = userName + "|" + fileName + "|" + masterKey + "\n";
                    logger.info("Appended to the file" + str);
                    bufferWritter.write(str);
                }

            } else {

                //mount a volume
                logger.info("Volume is present.");
                mount(userName, fileName, masterKey, admPassword);

                logger.info("Volume is mounted now");

                FileWriter fileWritter = new FileWriter(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name, true);
                try (BufferedWriter bufferWritter = new BufferedWriter(fileWritter)) {
                    bufferWritter.write(userName + "|" + fileName + "|" + masterKey + "\n");
                    String str = userName + "|" + fileName + "|" + masterKey;
                    logger.info("Appended to the file" + str);
                }
            }
        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            logger.info("exiting the method");
            return -1;
        }

        dismount(userName, fileName, masterKey, admPassword);
        logger.info("Volume dismounted");
        logger.info("exiting the method");
        return 0;
    }

    public static int validateKey(String userName, String fileName, String masterKey, String admPassword) {

        File file;
        file = new File(PORPropertyConfigurator.file_to_be_mounted);
        String sCurrentLine = null;
        BufferedReader br = null;

        try {

            if (!file.exists()) {
                logger.info("Mounted partition not found. Please contact administrator" + PORPropertyConfigurator.file_to_be_mounted);
                return -1;
            }

            mount(userName, fileName, masterKey, admPassword);
            logger.info("partition mounted");

            try {

                br = new BufferedReader(new FileReader(PORPropertyConfigurator.mount_path + "/" + PORPropertyConfigurator.keystore_file_name));

                while ((sCurrentLine = br.readLine()) != null) {
                    logger.info(sCurrentLine);

                    if (sCurrentLine.startsWith(userName)) {
                        String array[] = sCurrentLine.split("\\|");
                        if (array[0].equalsIgnoreCase(userName)
                                && array[1].contains(fileName)
                                && array[2].equalsIgnoreCase(masterKey)) {
                            return 0;
                        }
                    }
                }
            } catch (Exception e) {
                logger.info(e.getMessage());
                logger.info("Exiting the method");
                return 0;
            }

        } catch (Exception e) {
            logger.info(e.getMessage());
            logger.info("Exiting the method");
            return 0;
        }

        dismount(userName, fileName, masterKey, admPassword);
        logger.info("Exiting the method");
        return 1;
    }

    public static boolean mount(String userName, String fileName, String masterKey, String admPassword) {
        String line = null;
        try {
            logger.info("Entering method");
            String command = "truecrypt " + PORPropertyConfigurator.file_to_be_mounted + " " + PORPropertyConfigurator.mount_path + " -k=" + PORPropertyConfigurator.key_file + " -p=" + masterKey + " --protect-hidden=no --password=" + PORPropertyConfigurator.truecrypt_password;
            logger.info(command);
            Process p = Runtime.getRuntime().exec(command);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            logger.info("Output of the process - mount Volume");

            PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(p.getOutputStream())), true);
            out.println(admPassword);
            out.println();

            while ((line = stdInput.readLine()) != null) {
                logger.info(line);
            }

            logger.info("End of output");

            BufferedReader stdError = new BufferedReader(new InputStreamReader(p.getErrorStream()));

            logger.info("ERROR in the process(if any) - mount volume");
            while ((line = stdError.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of error");

            int exitVal = p.waitFor();
            logger.info("Process exitValue: " + exitVal);

            p.destroy();

        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            return false;
        }
        return true;
    }

    public static boolean dismount(String userName, String fileName, String masterKey, String passString) {
        String line = null;
        logger.info("Entering the method");
        try {
            String command = "truecrypt -d " + PORPropertyConfigurator.mount_path;
            logger.info(command);
            Process p = Runtime.getRuntime().exec("truecrypt -d " + PORPropertyConfigurator.mount_path);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            logger.info("Output of the process - dismount Volume");


            PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(p.getOutputStream())), true);
            out.println(passString);
            logger.info(passString);

            while ((line = stdInput.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of output");

            BufferedReader stdError = new BufferedReader(new InputStreamReader(p.getErrorStream()));

            logger.info("ERROR in the process - dismount volume(if any)");
            while ((line = stdError.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of error");

            int exitVal = p.waitFor();
            logger.info("Process exitValue: " + exitVal);

            p.destroy();
        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the process");
            return false;
        }
        logger.info("Exiting the process");
        return true;
    }
}