
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author PoojaD
 */
public class KeyStore {

    public static String file_to_be_mounted = "/home/poojad/adu.txt";
    public static String op = "/home/poojad/x.txt";
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
                String command = "truecrypt --size=" + default_partition_size_MB + " --password=" + truecrypt_password + " -k \"\" --random-source=" + random_file + " --volume-type=normal --encryption=AES --hash=SHA-512 --filesystem=FAT " + "-c " + file_to_be_mounted + " > " + op;

                Process p = Runtime.getRuntime().exec(command);
                System.out.println(command);
                BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getErrorStream()));
                p.waitFor();
                p.destroy();

                for (int i = 0; i < 15; i++) {
                    line = stdInput.readLine();
                    System.out.println(line);
                }

                while ((line = stdInput.readLine()) != null) {
                    System.out.println(line);
                }

                //mount a volume
                mount(userName, fileName, masterKey);

                System.out.println("Volume mounted");
                keyFile = new File(mount_path + "/"+key_store);
                if (!file.exists()) {
                    file.createNewFile();
                    System.out.println(key_store + " created");
                }

                FileWriter fileWritter = new FileWriter(mount_path + "/"+key_store, true);
                BufferedWriter bufferWritter = new BufferedWriter(fileWritter);
                String str = userName + "|" + fileName + "|" + masterKey;
                System.out.println("Appended to the file" + str);
                bufferWritter.write(str);
                bufferWritter.close();

                System.out.println("In storeKey -3");

            } else {

                //mount a volume
                //mount(userName, fileName, masterKey);
                System.out.println("Volume is present.");
                mount(userName, fileName, masterKey);

                System.out.println("Volume is mounted now");

                FileWriter fileWritter = new FileWriter(mount_path + "/"+key_store, true);
                BufferedWriter bufferWritter = new BufferedWriter(fileWritter);
                bufferWritter.write(userName + "|" + fileName + "|" + masterKey);
                String str = userName + "|" + fileName + "|" + masterKey;
                System.out.println("Appended to the file" + str);
                bufferWritter.close();
            }
        } catch (IOException | InterruptedException ioe) {
           System.out.println(ioe.getMessage());
            return -1;
        }

        dismount(userName, fileName, masterKey);
        System.out.println("Volume dismounted");
        return 0;
    }

    public static int validateKey(String userName, String fileName, String masterKey) {

        File file;
        file = new File(file_to_be_mounted);
        String sCurrentLine = null;
        BufferedReader br = null;

        try {

            if (!file.exists()) {
                System.out.println("Mounted partition not found. Please contact administrator");
                return -1;
            }

            mount(userName, fileName, masterKey);
            System.out.println("partition mounted");

            try {

                br = new BufferedReader(new FileReader(mount_path + "/"+key_store));

                while ((sCurrentLine = br.readLine()) != null) {
                    System.out.println(sCurrentLine);
                }

                if (sCurrentLine.contains(userName)) {
                    String array[] = sCurrentLine.split("|");
                    if (array[2].equalsIgnoreCase(masterKey)) {
                        return 0;
                    } else {
                        return -1;
                    }
                }
            } catch (Exception e) {
               System.out.println(e.getMessage());
            }

        } catch (Exception e) {
           System.out.println(e.getMessage());
        }

        return 0;

    }

    public static boolean mount(String userName, String fileName, String masterKey) {
        String line = null;
        try {
            System.out.println("I am in mount");
            String command = "truecrypt " + file_to_be_mounted + " " + mount_path;
            System.out.println(command);
            Process p = Runtime.getRuntime().exec(command);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = stdInput.readLine()) != null) {
                System.out.println(line);
            }
            p.waitFor();
            p.destroy();
        } catch (IOException | InterruptedException ioe) {
            System.out.println(ioe.getMessage());
            return false;
        }
        return true;
    }

    public static boolean dismount(String userName, String fileName, String masterKey) {
        String line = null;
        try {
            String command = "\"truecrypt -d \" + mount_path";
            System.out.println("In dismount" + command);
            Process p = Runtime.getRuntime().exec("truecrypt -d " + mount_path);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = stdInput.readLine()) != null) {
                System.out.println(line);
            }
            p.waitFor();
            p.destroy();
        } catch (IOException | InterruptedException ioe) {
            System.out.println(ioe.getMessage());
            return false;
        }
        return true;
    }
}
