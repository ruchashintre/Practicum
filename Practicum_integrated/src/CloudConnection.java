/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author poojad
 */
public interface CloudConnection {
    
  
    
    public int connectToCloud( String pemFilePath,String accessKey,String secretKey);
    
    public int fileUpload(String hostName, String pemFilePath, String fileToUploadPath);
    
    public int fileDownload(String hostName, String pemFilePath, String filNameToBeDownloaded, String saveToLocation);
    
    public int verify(String hostName, String pemFilePath, String fileName);
    
}
