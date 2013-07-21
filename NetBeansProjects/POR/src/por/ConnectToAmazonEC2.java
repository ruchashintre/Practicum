package por;

import com.amazonaws.AmazonClientException;
import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.services.ec2.AmazonEC2Client;
import com.amazonaws.services.ec2.model.DescribeInstanceStatusRequest;
import com.amazonaws.services.ec2.model.DescribeInstanceStatusResult;
import com.amazonaws.services.ec2.model.DescribeInstancesRequest;
import com.amazonaws.services.ec2.model.DescribeInstancesResult;
import com.amazonaws.services.ec2.model.Instance;
import com.amazonaws.services.ec2.model.InstanceStatus;
import com.amazonaws.services.ec2.model.Reservation;
import com.amazonaws.services.ec2.model.RunInstancesRequest;
import com.amazonaws.services.ec2.model.RunInstancesResult;
import com.amazonaws.services.ec2.model.StartInstancesRequest;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;
import java.util.logging.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author poojad
 */
public class ConnectToAmazonEC2 {

    static Logger logger = Logger.getLogger(ConnectToAmazonEC2.class);
    protected static String pubDnsName = null;
    private static Process prcs = null;

    public static int connectToCloud(String pemFilePath, String accessKey, String secretKey) {
        //INPUT USER ACCOUNT DATA
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
        logger.info("Entering the class");
        try {
            AWSCredentials awsc = new BasicAWSCredentials(accessKey, secretKey);
            AmazonEC2Client ec2client = new AmazonEC2Client(awsc);

            String[] temp = pemFilePath.split("/");
            String keyName = temp[temp.length - 1];
            keyName = keyName.substring(0, keyName.length() - 4);
            logger.info("keyname " + keyName);

            DescribeInstancesRequest request = new DescribeInstancesRequest();
            DescribeInstancesResult result = ec2client.describeInstances(request);

            List<Reservation> reservations = result.getReservations();

            for (Reservation reservation : reservations) {
                List<Instance> instances = reservation.getInstances();

                for (Instance instance : instances) {

                    if (instance.getImageId().equals(PORPropertyConfigurator.ami_id)) {

                        String instanceName = instance.getState().getName();
                        pubDnsName = instance.getPublicDnsName();
                        logger.info("Public DnS Name " + pubDnsName);
                        logger.info("IP address of the instance " + instance.getPublicIpAddress());

                        if (instanceName.equals("stopped")) {

                            StartInstancesRequest startInstancesRequest = new StartInstancesRequest();
                            List<String> instanceList = new ArrayList<>();
                            instanceList.add(instance.getInstanceId());
                            startInstancesRequest.setInstanceIds(instanceList);
                            ec2client.startInstances(startInstancesRequest);
                            do {
                                logger.info("Instance Status = " + instance.getState().getName());
                                Thread.sleep(5 * 1000);
                                instanceName = instance.getState().getName();
                            } while (!instanceName.equals("running"));
                            logger.info("Exiting the method");
                            return 0;
                        } else if (instanceName.equalsIgnoreCase("running")) {
                            logger.info("Status = " + instance.getState().getName());
                            logger.info("Exiting the method");
                            return 0;
                        }
                    }
                }
            }

            // RUN EC2 INSTANCES
            RunInstancesRequest runInstancesRequest = new RunInstancesRequest();

            runInstancesRequest.withImageId(PORPropertyConfigurator.ami_id)
                    .withInstanceType("t1.micro")
                    .withMinCount(1)
                    .withMaxCount(1)
                    .withKeyName(keyName)
                    .withSecurityGroups("default");

            RunInstancesResult runResult = ec2client.runInstances(runInstancesRequest);

            // DISPLAY IF RUNNING OR NOT
            String status = null;
            String runningId = null;
            do {
                List<Instance> newlyCreatedList = runResult.getReservation().getInstances();
                Instance runningInstance = newlyCreatedList.get(0);
                runningId = runningInstance.getInstanceId();
                pubDnsName = runningInstance.getPublicDnsName();
                logger.info("Public DNS Name" + pubDnsName);
                logger.info("IP Address of the instance" + runningInstance.getPublicIpAddress());
                logger.info("ID of instance" + runningId);

                DescribeInstanceStatusRequest describeInstanceRequest = new DescribeInstanceStatusRequest().withInstanceIds(runningId);
                DescribeInstanceStatusResult describeInstanceResult = ec2client.describeInstanceStatus(describeInstanceRequest);
                List<InstanceStatus> state = describeInstanceResult.getInstanceStatuses();
                while (state.size() < 1) {
                    // Do nothing, just wait, have thread sleep if needed
                    describeInstanceResult = ec2client.describeInstanceStatus(describeInstanceRequest);
                    state = describeInstanceResult.getInstanceStatuses();
                }

                status = state.get(0).getInstanceState().getName();

                logger.info("Status = " + status);
            } while (!status.equals("running"));
            request = new DescribeInstancesRequest().withInstanceIds(runningId);
            result = ec2client.describeInstances(request);

            reservations = result.getReservations();

            for (Reservation reservation : reservations) {
                List<Instance> instances = reservation.getInstances();
                for (Instance instance : instances) {
                    pubDnsName = instance.getPublicDnsName();
                    logger.info("DNS " + pubDnsName);
                }
            }
            //pending, running, shutting-down, terminated, stopping, stopped
            logger.info("Status of the instance = " + status);

            //RETRIEVE A LIST OF ALL FILES
        } catch (AmazonClientException | InterruptedException e) {
            logger.info(e.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");
        return 0;
    }

    public static int fileUpload(String fileToUploadPath, String pemFilePath, String userName) {
        try {
            logger.info("Entering the method");
            String line = null;
            Runtime rt = Runtime.getRuntime();
            sshToCloud(pemFilePath);
            //create directory for each user, and then store it into the directory

            PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);

            logger.info("Creating a directory");
            String command = "mkdir -p " + userName;
            logger.info(command);
            out.println(command);
            out.println("exit");

            BufferedReader stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

            logger.info("ERROR in the process - create directory(if any)");
            while ((line = stdError.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of error");


            BufferedReader stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
            logger.info("Output of the process - create Directory");
            while ((line = stdInput.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of output");

            command = "scp -i " + pemFilePath + " " + fileToUploadPath + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + ":~" + "/" + userName;
            logger.info(command);
            prcs = rt.exec(command);
            stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
            logger.info("Output of the process - file upload");
            while ((line = stdInput.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of output");

            stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

            logger.info("ERROR in the process -file upload(if any)");
            while ((line = stdError.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of error");

            int exitVal = prcs.waitFor();
            logger.info("Process exitValue: " + exitVal);

            prcs.destroy();

        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");
        return 0;

    }

    public static int fileDownload(String userName, String pemFilePath, String filNameToBeDownloaded, String saveTo) {
        String line;
        try {
            logger.info("Entering the method");
            Runtime rt = Runtime.getRuntime();
            String command = "scp -i " + pemFilePath + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + ":~/" + userName + "/" + filNameToBeDownloaded + " " + saveTo + "/";
            logger.info(command);
            prcs = rt.exec(command);

            BufferedReader stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

            logger.info("ERROR in the process - create volume(if any)");
            while ((line = stdError.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of error");


            BufferedReader stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
            logger.info("Output of the process - create Volume");

            while ((line = stdInput.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of output");

            int exitVal = prcs.waitFor();
            logger.info("Process exitValue: " + exitVal);

            prcs.destroy();

        } catch (InterruptedException | IOException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");

        return 0;
    }

    public static int verify(String pemFilePath, String fileName, String userName) {

        logger.info("In the start of verify");
        try {
            Runtime rt = Runtime.getRuntime();
            prcs = null;
            String line = null;

            String command;
            command = "scp -i " + pemFilePath + " " + PORPropertyConfigurator.executable_path + PORPropertyConfigurator.por_server_executable + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + ":~";
            prcs = rt.exec(command);

            BufferedReader stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

            logger.info("ERROR in the process - create volume(if any)");
            while ((line = stdError.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of error");


            BufferedReader stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
            logger.info("Output of the process - create Volume");

            while ((line = stdInput.readLine()) != null) {
                logger.info(line);
            }
            logger.info("End of output");

            int exitVal = prcs.waitFor();
            logger.info("Process exitValue: " + exitVal);

            int status = sshToCloud(pemFilePath);
            if (status == 0) {
                try {
                    command = "pkill -9 -f porserver";
                    PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);
                    out.println(command);
                    command = "./porserver " + "~/" + userName + "/" + fileName + " &";
                    logger.info(command);

                    out.println(command);
                    out.println("exit");
                    Scanner in = new Scanner(prcs.getInputStream());
                    while (in.hasNextLine()) {
                        logger.info(in.nextLine());
                    }
                } catch (Exception ioe) {
                    logger.info(ioe.getMessage());
                    logger.info("Exiting the method");
                    return -1;
                }
            }
            prcs.destroy();

        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;

        }
        logger.info("Exiting the method");

        return 0;
    }

    public static int sshToCloud(String pemFilePath) {

        logger.info("Entering the method");
        try {
            Runtime rt = Runtime.getRuntime();
            String command;
            command = "pkill -9 -f ssh";
            prcs = rt.exec(command);
            prcs.waitFor();
            prcs.destroy();
            command = "ssh -t -t -i " + pemFilePath + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + " \'/bin/bash\'";
            logger.info(command);
            prcs = rt.exec(command);


            logger.info("SSHed into instance");

        } catch (Exception ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");

        return 0;
    }

    public static ArrayList<String> getFileList(String pemFilePath, String userName) {

        logger.info("In the start of getFileList");

        ArrayList<String> list = new ArrayList<>();
        prcs = null;
        int status = sshToCloud(pemFilePath);
        String line;

        logger.info(status);

        if (status == 0) {
            try {

                String command = "cd " + userName + "; ls";
                logger.info(command);

                PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);
                BufferedReader stdInput = new BufferedReader(new InputStreamReader(prcs.getInputStream()));
                BufferedReader stdError = new BufferedReader(new InputStreamReader(prcs.getErrorStream()));

                out.println(command);
                out.println("exit");

                logger.info("Output of the process - get file list");

                while ((line = stdInput.readLine()) != null) {
                    logger.info(line);
                    if (line.endsWith("ls")) {
                        while (!(line = stdInput.readLine()).endsWith("exit")) {
                            String[] lines = line.split("\\s+");
                            list.addAll(Arrays.asList(lines));

                        }
                    }
                }
            } catch (IOException ex) {
                logger.info(ex.getMessage());
                logger.info("Exiting the method");
                return null;
            }
            logger.info("End of output");
            logger.info("List of files" + list.toString());


            int exitVal;
            try {
                exitVal = prcs.waitFor();
                logger.info("Process exitValue: " + exitVal);

            } catch (InterruptedException ex) {
                java.util.logging.Logger.getLogger(ConnectToAmazonEC2.class.getName()).log(Level.SEVERE, null, ex);
            }

            prcs.destroy();

        } else {
            logger.info("can not SSH into cloud");
            return null;
        }
        logger.info("Exiting the method");
        return list;
    }
}
