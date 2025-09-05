// 快速测试版本 - 直接运行
public class QuickTest {
    public static void main(String[] args) {
        System.out.println("=== FastLanes Java 性能测试 ===");
        
        // 测试数据
        int size = 100000;
        float[] data = new float[size];
        for (int i = 0; i < size; i++) {
            data[i] = 100.0f + (float) (Math.sin(i * 0.1) * 10.0);
        }
        
        // 测试Float32
        testFloat32(data);
        
        // 测试Float64
        double[] data64 = new double[size];
        for (int i = 0; i < size; i++) {
            data64[i] = 100.0 + Math.sin(i * 0.1) * 10.0;
        }
        testFloat64(data64);
    }
    
    private static void testFloat32(float[] data) {
        long start = System.nanoTime();
        byte[] encoded = encodeFloat32(data);
        long encodeTime = System.nanoTime() - start;
        
        start = System.nanoTime();
        float[] decoded = decodeFloat32(encoded, data.length);
        long decodeTime = System.nanoTime() - start;
        
        double ratio = (double) encoded.length / (data.length * 4);
        double encodeMBs = (data.length * 4.0) / (encodeTime / 1e9) / 1024 / 1024;
        double decodeMBs = (data.length * 4.0) / (decodeTime / 1e9) / 1024 / 1024;
        
        System.out.printf("Float32 - 编码: %.2fms (%.1f MB/s), 解码: %.2fms (%.1f MB/s), 压缩比: %.2f%%\n",
                encodeTime / 1e6, encodeMBs, decodeTime / 1e6, decodeMBs, ratio * 100);
    }
    
    private static void testFloat64(double[] data) {
        long start = System.nanoTime();
        byte[] encoded = encodeFloat64(data);
        long encodeTime = System.nanoTime() - start;
        
        start = System.nanoTime();
        double[] decoded = decodeFloat64(encoded, data.length);
        long decodeTime = System.nanoTime() - start;
        
        double ratio = (double) encoded.length / (data.length * 8);
        double encodeMBs = (data.length * 8.0) / (encodeTime / 1e9) / 1024 / 1024;
        double decodeMBs = (data.length * 8.0) / (decodeTime / 1e9) / 1024 / 1024;
        
        System.out.printf("Float64 - 编码: %.2fms (%.1f MB/s), 解码: %.2fms (%.1f MB/s), 压缩比: %.2f%%\n",
                encodeTime / 1e6, encodeMBs, decodeTime / 1e6, decodeMBs, ratio * 100);
    }
    
    private static byte[] encodeFloat32(float[] values) {
        byte[] encoded = new byte[values.length * 4 + 4];
        int first = Float.floatToIntBits(values[0]);
        encoded[0] = (byte) first; encoded[1] = (byte) (first >> 8);
        encoded[2] = (byte) (first >> 16); encoded[3] = (byte) (first >> 24);
        
        int pos = 4;
        for (int i = 1; i < values.length; i++) {
            int current = Float.floatToIntBits(values[i]);
            int prev = Float.floatToIntBits(values[i-1]);
            int xor = current ^ prev;
            encoded[pos++] = (byte) xor; encoded[pos++] = (byte) (xor >> 8);
            encoded[pos++] = (byte) (xor >> 16); encoded[pos++] = (byte) (xor >> 24);
        }
        return java.util.Arrays.copyOf(encoded, pos);
    }
    
    private static float[] decodeFloat32(byte[] data, int len) {
        float[] values = new float[len];
        int first = ((data[0] & 0xFF) | ((data[1] & 0xFF) << 8) |
                    ((data[2] & 0xFF) << 16) | ((data[3] & 0xFF) << 24));
        values[0] = Float.intBitsToFloat(first);
        
        int pos = 4;
        for (int i = 1; i < len; i++) {
            int xor = ((data[pos] & 0xFF) | ((data[pos+1] & 0xFF) << 8) |
                      ((data[pos+2] & 0xFF) << 16) | ((data[pos+3] & 0xFF) << 24));
            int prev = Float.floatToIntBits(values[i-1]);
            values[i] = Float.intBitsToFloat(prev ^ xor);
            pos += 4;
        }
        return values;
    }
    
    private static byte[] encodeFloat64(double[] values) {
        byte[] encoded = new byte[values.length * 8 + 8];
        long first = Double.doubleToLongBits(values[0]);
        for (int i = 0; i < 8; i++) encoded[i] = (byte) (first >> (i * 8));
        
        int pos = 8;
        for (int i = 1; i < values.length; i++) {
            long current = Double.doubleToLongBits(values[i]);
            long prev = Double.doubleToLongBits(values[i-1]);
            long xor = current ^ prev;
            for (int j = 0; j < 8; j++) encoded[pos++] = (byte) (xor >> (j * 8));
        }
        return java.util.Arrays.copyOf(encoded, pos);
    }
    
    private static double[] decodeFloat64(byte[] data, int len) {
        double[] values = new double[len];
        long first = 0;
        for (int i = 0; i < 8; i++) first |= ((long) (data[i] & 0xFF)) << (i * 8);
        values[0] = Double.longBitsToDouble(first);
        
        int pos = 8;
        for (int i = 1; i < len; i++) {
            long xor = 0;
            for (int j = 0; j < 8; j++) xor |= ((long) (data[pos+j] & 0xFF)) << (j * 8);
            long prev = Double.doubleToLongBits(values[i-1]);
            values[i] = Double.longBitsToDouble(prev ^ xor);
            pos += 8;
        }
        return values;
    }
}