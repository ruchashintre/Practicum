import java.util.List;

import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.services.ec2.AmazonEC2Client;
import com.amazonaws.services.ec2.model.AuthorizeSecurityGroupIngressRequest;
import com.amazonaws.services.ec2.model.CreateKeyPairRequest;
import com.amazonaws.services.ec2.model.CreateKeyPairResult;
import com.amazonaws.services.ec2.model.CreateSecurityGroupRequest;
import com.amazonaws.services.ec2.model.CreateSecurityGroupResult;
import com.amazonaws.services.ec2.model.DescribeInstanceStatusRequest;
import com.amazonaws.services.ec2.model.DescribeInstanceStatusResult;
import com.amazonaws.services.ec2.model.EbsBlockDevice;
import com.amazonaws.services.ec2.model.InstanceState;
import com.amazonaws.services.ec2.model.InstanceStatus;
import com.amazonaws.services.ec2.model.IpPermission;
import com.amazonaws.services.ec2.model.RunInstancesRequest;
import com.amazonaws.services.ec2.model.RunInstancesResult;

public class connect 
{
	public static void main(String[] args)
	{
		//INPUT USER ACCOUNT DATA
		
		//AWSCredentialsProvider credentialsProvider = new ClasspathPropertiesFileCredentialsProvider();
		//ClasspathPropertiesFileCredentialsProvider(String credentialsFilePath)
		String accessKey = "";
		String secretKey = "";
		AWSCredentials awsc = new BasicAWSCredentials(accessKey, secretKey); 
		AmazonEC2Client ec2client = new AmazonEC2Client(awsc);
		
		//CREATE SECURITY GROUP
		CreateSecurityGroupRequest csgrequest = new CreateSecurityGroupRequest();
			        	
		csgrequest.withGroupName("PORGroup")
				.withDescription("Security Group for POR");
		
		CreateSecurityGroupResult csgresult =ec2client.createSecurityGroup(csgrequest);
		
		//ENALBLE INBOUND TRAFFIC
		IpPermission ipPermission = new IpPermission();
			    	
		ipPermission.withIpRanges("111.111.111.111/32", "150.150.150.150/32")
			            .withIpProtocol("tcp")
			            .withFromPort(22)
			            .withToPort(22);
		
		AuthorizeSecurityGroupIngressRequest authorizeSecurityGroupIngressRequest = new AuthorizeSecurityGroupIngressRequest();
			    	
		authorizeSecurityGroupIngressRequest.withGroupName("JavaSecurityGroup")
			                                    .withIpPermissions(ipPermission);
		
		ec2client.authorizeSecurityGroupIngress(authorizeSecurityGroupIngressRequest);
		
		// CREATE KEY PAIR
		CreateKeyPairRequest createKeyPairRequest = new CreateKeyPairRequest();
				    	
		createKeyPairRequest.withKeyName(keyName);
		
		CreateKeyPairResult createKeyPairResult = amazonEC2Client.createKeyPair(createKeyPairRequest);
		
		
		// RUN EC2 INSTANCES
		RunInstancesRequest runInstancesRequest = 
				  new RunInstancesRequest();
			        	
		runInstancesRequest.withImageId("ami-4b814f22")
			                     .withInstanceType("m1.small")
			                     .withMinCount(1)
			                     .withMaxCount(1)
			                     .withKeyName("YourKeyName")
			                     .withSecurityGroups("YourSecurityGroupName");
		
		 RunInstancesResult runInstancesResult = ec2client.runInstances(runInstancesRequest);
		
		// DISPLAY IF RUNNING OR NOT
		DescribeInstanceStatusRequest disrequest = new DescribeInstanceStatusRequest();
		disrequest.withIncludeAllInstances(true);
		DescribeInstanceStatusResult disresult = describeInstanceStatus(disrequest);
		
		List<InstanceStatus> list = disresult.getInstanceStatuses();
		InstanceStatus istatus = list.get(0);
		InstanceState istate = istatus.getInstanceState();
		String status = istate.getName();
		//pending, running, shutting-down, terminated, stopping, stopped
		System.out.println("Status of the instance = " + status);
		
		//CREATE EBS
		EbsBlockDevice ebd = new EbsBlockDevice();
		
		ebd.withDeleteOnTermination(false)
		.withIops(3)
		.withSnapshotId("")
		.withVolumeSize(16)
		.withVolumeType("standard")
		;
		
		//RETRIEVE A LIST OF ALL FILES
	}
}
