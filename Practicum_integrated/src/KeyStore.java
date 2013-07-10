
import java.io.File;
import java.io.IOException;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.FileReader;
/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author PoojaD
 */
public class KeyStore {

    public static String file_to_be_mounted = "/home/poojad/a.txt";
    public static String mount_path = "/media/DISK1";
    public static int default_partition_size_MB = 1074000;
    public static String truecrypt_password = "poojad";
    public static String random_file = "/home/poojad/random";
    public static String key_store = "keyStore.txt";

    public static int storeKey(String userName, String fileName, String masterKey) {


        String line = null;
        File file = null;
        File keyFile = null;

        file = new File(file_to_be_mounted);

        try {
            //if file doesnt exists, then create it
            if (!file.exists()) {
                file.createNewFile();

                //create the volume
                //check my volume .tc
                String command = "truecrypt -t --size=" + default_partition_size_MB + " --password=" + truecrypt_password + " --random-source=" + random_file + " --volume-type=normal --encryption=AES --hash=SHA-512 --filesystem=FAT " + "-c " + file_to_be_mounted;
                Process p = Runtime.getRuntime().exec(command);
                System.out.println(command);
                BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
                while ((line = stdInput.readLine()) != null) {
                    System.out.println(line);
                }

                p = Runtime.getRuntime().exec("");
                //mount a volume
                mount(userName, fileName, masterKey);

                keyFile = new File(key_store);
                if (!file.exists()) {
                    file.createNewFile();
                }

                FileWriter fileWritter = new FileWriter(key_store, true);
                BufferedWriter bufferWritter = new BufferedWriter(fileWritter);
                bufferWritter.write(userName + "|" + fileName + "|" + masterKey);
                bufferWritter.close();

                System.out.println("In storeKey -3");

            } else {
                
                //mount a volume
                mount(userName, fileName, masterKey);

                FileWriter fileWritter = new FileWriter(key_store, true);
                BufferedWriter bufferWritter = new BufferedWriter(fileWritter);
                bufferWritter.write(userName + "|" + fileName + "|" + masterKey);
                bufferWritter.close();
            }
        } catch (IOException ioe) {
            ioe.printStackTrace();
            return -1;
        }


        /*File Upload
         1. if first time
         - create a volume
         2. else
         - mount the volume
         - open the file
         - append entry to it at the end : useid, file name, password
         - close the file
         - dismount the volume
         */
        return 0;
    }

    public static int validateKey(String userName, String fileName, String masterKey) {

        File file;
        file = new File(file_to_be_mounted);

        try {

            if (!file.exists()) {
                System.out.println("Mounted partition not found. Please contact administrator");
                return -1;
            }

            mount(userName, fileName, masterKey);
            BufferedReader br = null;

            try {

                String sCurrentLine;

                br = new BufferedReader(new FileReader("C:\\testing.txt"));

                while ((sCurrentLine = br.readLine()) != null) {
                    System.out.println(sCurrentLine);
                }

                if (sCurrentLine.contains(userName)) {
                    String array[] = sCurrentLine.split("|");
                    if (array[1].equalsIgnoreCase(masterKey)) {
                        return 0;
                    } else {
                        return -1;
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            ////

        } catch (Exception e) {
            e.printStackTrace();
        }

        return 0;

    }

    public static boolean mount(String userName, String fileName, String masterKey) {
        String line = null;
        try {
            System.out.println("I am in mount");
            Process p = Runtime.getRuntime().exec("truecrypt " + file_to_be_mounted + " " + mount_path);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = stdInput.readLine()) != null) {
                System.out.println(line);
            }
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
        return false;
    }

    public static boolean dismount(String userName, String fileName, String masterKey) {
        String line = null;
        try {
            Process p = Runtime.getRuntime().exec("truecrypt -d " + mount_path);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = stdInput.readLine()) != null) {
                System.out.println(line);
            }
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
        return false;
    }
}
