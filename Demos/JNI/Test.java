import java.util.*;
public class Test {    
    public static void main(String[] args) {
    	try{
	    	Runtime rt = Runtime.getRuntime();
	    	Process pr = rt.exec("./a.out");
	    	pr.waitFor();
	    	}catch(Exception e) {
	    		System.out.println(e.toString());
	    	}
    	}
}
