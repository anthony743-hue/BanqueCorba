import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;

public class FileHandler {
    private static String defaultSeparator = ",";

    public static void readFile(String filePath, ArrayList<Object> dataList, Object targetObject)
            throws Exception {

        Path path = Paths.get(filePath);
        Class<?> classTarget = targetObject.getClass();
        String pattern = "set%s";

        try (BufferedReader br = new BufferedReader(new FileReader(path.toFile()))) {

            // 1) Lire et ignorer l'en-tête
            String headerLine = br.readLine();
            if (headerLine == null) return;
            String[] headers = headerLine.split(defaultSeparator);

            // 2) Lire chaque ligne de données
            String line;
            while ((line = br.readLine()) != null) {
                String[] values = line.split(defaultSeparator);
                if(values.length != headers.length){
                    continue;
                }
                Object newInstance = classTarget.getDeclaredConstructor().newInstance();

                for(int i=0; i < headers.length; i++){
                    Field field = classTarget.getField(headers[i]);
                    Class<?> type = field.getType();
                    String nom = headers[i];

                    Method setter = classTarget.getMethod(
                            String.format(pattern, capitalized(nom)), type);

                    Object converted = convert(values[i].trim(), type);
                    setter.invoke(newInstance, converted);
                }
                dataList.add(newInstance);
            }

        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
    }

    /** Conversion String -> String | primitif | wrapper Java. */
    private static Object convert(String value, Class<?> type) {
        if (type == String.class || type == CharSequence.class) return value;

        if ((value == null || value.isEmpty()) && !type.isPrimitive()) return null;

        if (type == int.class     || type == Integer.class)   return Integer.parseInt(value);
        if (type == long.class    || type == Long.class)      return Long.parseLong(value);
        if (type == short.class   || type == Short.class)     return Short.parseShort(value);
        if (type == byte.class    || type == Byte.class)      return Byte.parseByte(value);
        if (type == double.class  || type == Double.class)    return Double.parseDouble(value);
        if (type == float.class   || type == Float.class)     return Float.parseFloat(value);
        if (type == boolean.class || type == Boolean.class)   return Boolean.parseBoolean(value);
        if (type == char.class    || type == Character.class)
            return value.isEmpty() ? '\0' : value.charAt(0);

        throw new IllegalArgumentException("Type non supporté : " + type.getName());
    }

    private static String capitalized(String s) {
        return Character.toUpperCase(s.charAt(0)) + s.substring(1);
    }
}