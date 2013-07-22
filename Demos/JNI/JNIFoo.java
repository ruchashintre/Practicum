public class JNIFoo {    
    public native int nativeFoo();    

    static {
        System.loadLibrary("foo");
    }        

    public void print () {
    int res = nativeFoo();
    }
    
    public static void main(String[] args) {
    (new JNIFoo()).print();
    return;
    }
}
