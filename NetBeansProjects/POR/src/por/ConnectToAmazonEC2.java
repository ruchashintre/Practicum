package por;

import por.util.PORPropertyConfigurator;
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
import com.amazonaws.services.ec2.model.StopInstancesRequest;
import com.amazonaws.services.ec2.model.Tag;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import por.util.CommandExecutor;
import static por.util.CommandExecutor.prcs;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author rucha, poojad
 */
public class ConnectToAmazonEC2 {

    private static Instance running_instance;
    private static AmazonEC2Client ec2client;
    static Logger logger = Logger.getLogger(ConnectToAmazonEC2.class);
    protected static String pubDnsName = null;

    public static int connectToCloud(String pemFilePath, String accessKey, String secretKey) {
        //INPUT USER ACCOUNT DATA
        PropertyConfigurator.configure(PORPropertyConfigurator.logger_path);
        logger.info("Entering the class");
        try {
            AWSCredentials awsc = new BasicAWSCredentials(accessKey, secretKey);
            ec2client = new AmazonEC2Client(awsc);

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
                        running_instance = instance;
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
                                DescribeInstanceStatusRequest describeInstanceRequest = new DescribeInstanceStatusRequest().withInstanceIds(instance.getInstanceId());
                                DescribeInstanceStatusResult describeInstanceResult = ec2client.describeInstanceStatus(describeInstanceRequest);
                                List<InstanceStatus> state = describeInstanceResult.getInstanceStatuses();
                                while (state.size() < 1) {
                                    // Do nothing, just wait, have thread sleep if needed
                                    describeInstanceResult = ec2client.describeInstanceStatus(describeInstanceRequest);
                                    state = describeInstanceResult.getInstanceStatuses();
                                }
                                instanceName = state.get(0).getInstanceState().getName();
                                logger.info("Instance Name=" + instanceName);
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
                running_instance = runningInstance;
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
        String command = null;
        logger.info("Entering the method");

        try {
            sshToCloud(pemFilePath);

            //create directory for each user, and then store it into the directory
            logger.info("Creating a directory");
            command = "mkdir -p " + userName;
            CommandExecutor.executeInTerminalCommandAndExit(command);

            command = "scp -i " + pemFilePath + " " + fileToUploadPath + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + ":~" + "/" + userName;
            CommandExecutor.executeCommandAndExit(command);

        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");
        return 0;
    }

    public static int fileDownload(String userName, String pemFilePath, String filNameToBeDownloaded, String saveTo) {
        String command = null;
        logger.info("Entering the method");

        try {
            command = "scp -i " + pemFilePath + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + ":~/" + userName + "/" + filNameToBeDownloaded + " " + saveTo + "/";
            CommandExecutor.executeCommandAndExit(command);

        } catch (InterruptedException | IOException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");
        return 0;
    }

    public static int verify(String pemFilePath, String fileName, String userName) {

        String command;
        logger.info("In the start of verify");
        try {
            command = "scp -i " + pemFilePath + " " + PORPropertyConfigurator.executable_path + PORPropertyConfigurator.por_server_executable + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + ":~";
            CommandExecutor.executeCommandAndExit(command);

            sshToCloud(pemFilePath);
            try {
                command = "pkill -9 -f porserver";
                CommandExecutor.executeInTerminalCommand(command);

                command = "./porserver " + "~/" + userName + "/" + fileName + " &";
                CommandExecutor.executeInTerminalCommandAndExit(command);

            } catch (IOException | InterruptedException ioe) {
                logger.info(ioe.getMessage());
                logger.info("Exiting the method");
                return -1;
            }

        } catch (IOException | InterruptedException ioe) {
            logger.info(ioe.getMessage());
            logger.info("Exiting the method");
            return -1;
        }
        logger.info("Exiting the method");
        return 0;
    }

    public static void sshToCloud(String pemFilePath) throws IOException, InterruptedException {

        logger.info("Enterin the method");

        String command = null;
        Runtime rt = Runtime.getRuntime();

        CommandExecutor.executeCommandAndExit("pkill -9 -f ssh");
        command = "ssh -t -t -i " + pemFilePath + " " + PORPropertyConfigurator.instance_user_name + "@" + pubDnsName + " \'/bin/bash\'";

        logger.info("Command to be executed = " + command);
        prcs = rt.exec(command);

        logger.info("Exiting the method");

    }

    public static ArrayList<String> getFileList(String pemFilePath, String userName) throws IOException, InterruptedException {

        String line = null;
        ArrayList<String> list = new ArrayList<>();
        int exitVal = 0;

        logger.info("In the start of getFileList");

        sshToCloud(pemFilePath);

        String command = "cd " + userName + "; ls";
        logger.info(command);

        PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(CommandExecutor.prcs.getOutputStream())), true);
        BufferedReader stdInput = new BufferedReader(new InputStreamReader(CommandExecutor.prcs.getInputStream()));
        BufferedReader stdError = new BufferedReader(new InputStreamReader(CommandExecutor.prcs.getErrorStream()));

        out.println(command);
        out.println("exit");

        logger.info("Output of the process - get file list");
        while ((line = stdError.readLine()) != null) {
            logger.info(line);
        }

        while ((line = stdInput.readLine()) != null) {
            logger.info(line);

            if (line.endsWith("ls")) {
                while (!(line = stdInput.readLine()).endsWith("exit")) {
                    if (line.contains("No such file or directory")) {
                        throw new RuntimeException("No such file or directory. Please enter user name again");
                    }
                    logger.info(line);
                    String[] lines = line.split("\\s+");
                    list.addAll(Arrays.asList(lines));
                }
            }

        }

        exitVal = CommandExecutor.prcs.waitFor();
        logger.info("Process exitValue: " + exitVal);
        CommandExecutor.prcs.destroy();

        logger.info("List of files" + list.toString());
        logger.info("Exiting the method");
        return list;
    }

    public static void stopInstance() throws InterruptedException {

        StopInstancesRequest stopInstancesRequest = new StopInstancesRequest();
        List<String> instanceList = new ArrayList<>();
        instanceList.add(running_instance.getInstanceId());
        stopInstancesRequest.setInstanceIds(instanceList);
        ec2client.stopInstances(stopInstancesRequest);
        String instanceState="";
        do {
            logger.info("Instance Status = " + running_instance.getState().getName());
            Thread.sleep(5 * 1000);
            DescribeInstancesRequest dis = new DescribeInstancesRequest();
            ArrayList<String> runningInstanceList = new ArrayList<String>();
            runningInstanceList.add(running_instance.getInstanceId());
            dis.setInstanceIds(runningInstanceList);
            DescribeInstancesResult disresult = ec2client.describeInstances(dis);
            List<Reservation> list = disresult.getReservations();

            System.out.println("-------------- status of instances -------------");
            for (Reservation res : list) {
                List<Instance> instancelist = res.getInstances();

                for (Instance instance : instancelist) {

                    logger.info("Instance Status : " + instance.getState().getName());
                    List<Tag> t1 = instance.getTags();
                    for (Tag teg : t1) {
                        logger.info("Instance Name   : " + teg.getValue());
                    }
                    instanceState = instance.getState().getName();
                    logger.info("Instance State = " + instanceState);
                }
            }
        } while (!instanceState.equals("stopped"));
        logger.info("Exiting the method");
    }
}