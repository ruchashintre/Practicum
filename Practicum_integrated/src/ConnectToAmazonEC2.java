
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
import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author poojad
 */
public class ConnectToAmazonEC2 {

    public static String pubDnsName = "";
    private static Process prcs = null;
    private static String ami = "ami-d0f89fb9";

    public static int connectToCloud(String pemFilePath, String accessKey, String secretKey) {
        //INPUT USER ACCOUNT DATA

        try {
            AWSCredentials awsc = new BasicAWSCredentials(accessKey, secretKey);
            AmazonEC2Client ec2client = new AmazonEC2Client(awsc);

            String[] temp = pemFilePath.split("/");
            String keyName = temp[temp.length - 1];
            keyName = keyName.substring(0, keyName.length() - 4);
            System.out.println("keyname " + keyName);

            DescribeInstancesRequest request = new DescribeInstancesRequest();
            DescribeInstancesResult result = ec2client.describeInstances(request);

            List<Reservation> reservations = result.getReservations();

            for (Reservation reservation : reservations) {
                List<Instance> instances = reservation.getInstances();

                for (Instance instance : instances) {

                    if (instance.getImageId().equals(ami)) {


                        String instanceName = instance.getState().getName();
                        pubDnsName = instance.getPublicDnsName();
                        System.out.println(pubDnsName);
                        System.out.println(instance.getPublicIpAddress());

                        if (instanceName.equals("stopped")) {

                            StartInstancesRequest startInstancesRequest = new StartInstancesRequest();
                            List<String> instanceList = new ArrayList<String>();
                            instanceList.add(instance.getInstanceId());
                            startInstancesRequest.setInstanceIds(instanceList);
                            ec2client.startInstances(startInstancesRequest);
                            do {
                                System.out.println("Status = " + instance.getState().getName());
                                Thread.sleep(5 * 1000);
                                instanceName = instance.getState().getName();
                            } while (!instanceName.equals("running"));
                            return 0;
                        } else if (instanceName.equalsIgnoreCase("running")) {
                            System.out.println("Status = " + instance.getState().getName());
                            return 0;
                        }
                    }
                }

            }

            // RUN EC2 INSTANCES
            RunInstancesRequest runInstancesRequest = new RunInstancesRequest();

            runInstancesRequest.withImageId(ami) // change to AMI of t1.micro
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
                System.out.println(pubDnsName);
                System.out.println(runningInstance.getPublicIpAddress());

                System.out.println(runningId);


                DescribeInstanceStatusRequest describeInstanceRequest = new DescribeInstanceStatusRequest().withInstanceIds(runningId);
                DescribeInstanceStatusResult describeInstanceResult = ec2client.describeInstanceStatus(describeInstanceRequest);
                List<InstanceStatus> state = describeInstanceResult.getInstanceStatuses();
                while (state.size() < 1) {
                    // Do nothing, just wait, have thread sleep if needed
                    describeInstanceResult = ec2client.describeInstanceStatus(describeInstanceRequest);
                    state = describeInstanceResult.getInstanceStatuses();
                }

                status = state.get(0).getInstanceState().getName();

                System.out.println("Status = " + status);
            } while (!status.equals("running"));
            request = new DescribeInstancesRequest().withInstanceIds(runningId);
            result = ec2client.describeInstances(request);

            reservations = result.getReservations();

            for (Reservation reservation : reservations) {
                List<Instance> instances = reservation.getInstances();
                for (Instance instance : instances) {
                    pubDnsName = instance.getPublicDnsName();
                    System.out.println("DNS " + pubDnsName);
                }
            }
            //pending, running, shutting-down, terminated, stopping, stopped
            System.out.println("Status of the instance = " + status);
            /*
             //CREATE EBS
             EbsBlockDevice ebd = new EbsBlockDevice();
		
             ebd.withDeleteOnTermination(false)
             .withIops(3)
             .withSnapshotId("")
             .withVolumeSize(16)
             .withVolumeType("standard")
             ;
             */
            //RETRIEVE A LIST OF ALL FILES
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }


        return 0;

    }

    public static int fileUpload(String fileToUploadPath, String pemFilePath) {
        try {
            Runtime rt = Runtime.getRuntime();
            String command = "scp -i " + pemFilePath + " " + fileToUploadPath + " ubuntu@" + pubDnsName + ":~";
            System.out.println(command);
            prcs = rt.exec(command);

            InputStreamReader isr = new InputStreamReader(prcs.getInputStream());
            BufferedReader br = new BufferedReader(isr);
            String line;
            while ((line = br.readLine()) != null) {
                System.out.println(line);
            }
            prcs.destroy();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            return -1;
        }
        return 0;
    }

    public static int fileDownload(String hostName, String pemFilePath, String filNameToBeDownloaded, String saveToLocation) {

        try {
            Runtime rt = Runtime.getRuntime();
            String command = "scp -i " + pemFilePath + " ubuntu@" + pubDnsName + ":~/" + filNameToBeDownloaded + " " + saveToLocation;
            System.out.println(command);
            prcs = rt.exec(command);
            InputStreamReader isr = new InputStreamReader(prcs.getInputStream());
            BufferedReader br = new BufferedReader(isr);
            String line;
            while ((line = br.readLine()) != null) {
                System.out.println(line);
            }
            prcs.destroy();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            return -1;
        }
        return 0;
    }

    public static int verify(String pemFilePath, String fileName) {

        try {
            Runtime rt = Runtime.getRuntime();
            prcs = null;

            int status = sshToCloud(pemFilePath, fileName);
            if (status == 0) {
                try {
                    System.out.println("In the start of verify");
                    String command = "pkill -9 -f porserver";
                    PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);
                    out.println(command);
                    command = "./porserver " + fileName;
                    System.out.println(command);

                    out.println(command);

                } catch (Exception ioe) {
                    ioe.printStackTrace();
                }
            }
            prcs.destroy();

        } catch (Exception ioe) {
            ioe.printStackTrace();
            return -1;
        }
        return 0;
    }

    public static int sshToCloud(String pemFilePath, String fileName) {

        System.out.println("in SSH");
        try {
            Runtime rt = Runtime.getRuntime();
            String command = "ssh -t -t -i " + pemFilePath + " ubuntu@" + pubDnsName + " \'/bin/bash\'";
            System.out.println(command);
            prcs = rt.exec(command);

            System.out.println("I am done");

        } catch (IOException ioe) {
            ioe.printStackTrace();
            return -1;
        }
        return 0;
    }

    public static ArrayList<String> getFileList(String pemFilePath, String fileName) {

        ArrayList<String> list = new ArrayList<String>();
        prcs = null;
        int status = sshToCloud(pemFilePath, fileName);
        String line = null;

        System.out.println(status);

        if (status == 0) {
            try {
                System.out.println("In the start of getFileList");


                String command = "ls";
                PrintWriter out = new PrintWriter(new OutputStreamWriter(new BufferedOutputStream(prcs.getOutputStream())), true);
                out.println(command);

                InputStreamReader isr = new InputStreamReader(prcs.getInputStream());
                BufferedReader br = new BufferedReader(isr);

                System.out.println("Before While");
                int i = 0;
                line = br.readLine();
                line = br.readLine();
                line = br.readLine();
                line = br.readLine();
                line = br.readLine();
                line = br.readLine();

                String[] lines = line.split("\\s+");
                System.out.println(line);
                for (String a : lines) {
                    list.add(a);
                }
                prcs.destroy();

                System.out.println("after while");
            } catch (IOException ioe) {
                ioe.printStackTrace();
                return null;
            }


        }
        return list;
    }
}
