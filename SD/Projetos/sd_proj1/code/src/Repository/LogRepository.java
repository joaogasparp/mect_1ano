package Repository;

import javax.swing.table.DefaultTableModel;
import java.io.FileWriter;
import java.io.IOException;

public class LogRepository {
    public synchronized void saveTableToFile(DefaultTableModel tableModel, String filename) {
        try (FileWriter writer = new FileWriter(filename)) {
            // Escreve os nomes das colunas
            for (int i = 0; i < tableModel.getColumnCount(); i++) {
                writer.write(tableModel.getColumnName(i));
                if (i < tableModel.getColumnCount() - 1) {
                    writer.write(";");
                }
            }
            writer.write("\n");

            // Escreve os dados da tabela
            for (int i = 0; i < tableModel.getRowCount(); i++) {
                for (int j = 0; j < tableModel.getColumnCount(); j++) {
                    writer.write(tableModel.getValueAt(i, j).toString());
                    if (j < tableModel.getColumnCount() - 1) {
                        writer.write(";");
                    }
                }
                writer.write("\n");
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
