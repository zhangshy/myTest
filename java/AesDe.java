/**
 * 使用AES CBC模式解密文件。使用CBC模式时要使用iv
 */
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.Random;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;


public class AesDe {

	public static void main(String[] args) {
		byte[] password = "Sixteen byte key".getBytes();
		try {
				InputStream fin = new FileInputStream("testEn.jar");
				OutputStream fos= new FileOutputStream("testDe4java.jar");
				SecretKeySpec key = new SecretKeySpec(password, "AES");
				Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
				byte[] ivByte = new byte[16];
				new Random().nextBytes(ivByte);
				System.out.printf("ivbytes:%x %x %x\n", ivByte[0], ivByte[1], ivByte[15]);
				IvParameterSpec iv = new IvParameterSpec(ivByte);
				cipher.init(2, key, iv);
				int totalLen = fin.available();
				byte[] byteContent = new byte[totalLen];
				int len = 0;
				len = fin.read(byteContent);
				if (len!=totalLen)
					System.out.println("read is not equal");
				byte[] result = cipher.doFinal(byteContent);
                //前四个字节是文件长度
                int fileLen = ((result[16]&0xff)<<24)+((result[17]&0xff)<<16)+((result[18]&0xff)<<8)+(result[19]&0xff);
				if ((fileLen+20)>len)
                    fos.write(result, 20, len-20);
                else
                    fos.write(result, 20, fileLen);
				fin.close();
				fos.close();
			} catch (NoSuchAlgorithmException e) {
			  e.printStackTrace();
			} catch (NoSuchPaddingException e) {
			  e.printStackTrace();
			} catch (InvalidKeyException e) {
			  e.printStackTrace();
			} catch (IllegalBlockSizeException e) {
			  e.printStackTrace();
			} catch (BadPaddingException e) {
			  e.printStackTrace();
			}catch (FileNotFoundException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InvalidAlgorithmParameterException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	}

}
