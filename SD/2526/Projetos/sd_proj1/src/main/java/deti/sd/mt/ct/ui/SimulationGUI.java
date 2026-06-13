package deti.sd.mt.ct.ui;

import deti.sd.mt.ct.core.ChargingStation;
import deti.sd.mt.ct.core.CityMap;
import deti.sd.mt.ct.core.Intersection;
import deti.sd.mt.ct.core.Vehicle;
import deti.sd.mt.ct.model.Direction;
import deti.sd.mt.ct.model.VehiclePriority;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.Path2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SimulationGUI {
    // configs visuais gerais (espaçamento, tamanho das celulas e dos veículos)
    private static final int CELL = 100;
    private static final int MARGIN = 70;
    private static final int V_SIZE = 14;

    // paleta de cores
    private static final Color BG = new Color(25, 25, 30); // bg
    private static final Color ROAD = new Color(50, 50, 55); // estrada
    private static final Color ISEC = new Color(80, 110, 150); // interseção livre
    private static final Color CHARGER = new Color(30, 200, 100); // plug
    private static final Color BUSY = new Color(240, 170, 0); // interseção ocupada
    private static final Color LOW_BAT = new Color(220, 50, 50); // bateria low
    private static final Color DIVERT = new Color(255, 130, 0); // desvio para carregar
    private static final Color CHARGING = new Color(0, 255, 120); // charging
    private static final Color EMERGENCY = new Color(255, 40, 40); // cor para veículos de emergência
    private static final Color DONE = new Color(90, 90, 95); // terminado

    // fontes para labels e texto
    private static final Font F_MAIN = new Font("SansSerif", Font.PLAIN, 11);
    private static final Font F_BOLD = new Font("SansSerif", Font.BOLD, 10);
    private static final Font F_MONO = new Font("Monospaced", Font.PLAIN, 12);

    private final CityMap map;
    private final List<Vehicle> vehicles;
    private final List<ChargingStation> stations = new ArrayList<>();

    private JLabel status; // barra de status inferior
    private final Map<String, VehicleRow> vRows = new HashMap<>(); // lista de veículos
    private final Map<Integer, StationRow> sRows = new HashMap<>(); // lista de plugs

    public SimulationGUI(CityMap map, List<Vehicle> vehicles) {
        this.map = map;
        this.vehicles = vehicles;

        // recolher todas as estações de carregamento do mapa para listar no painel
        for (int r = 0; r < map.getRows(); r++) {
            for (int c = 0; c < map.getCols(); c++) {
                Intersection isec = map.getIntersection(r, c);
                if (isec != null && isec.hasChargingStation()) {
                    stations.add(isec.getChargingStation());
                }
            }
        }
    }

    public void show() {
        SwingUtilities.invokeLater(() -> {
            JFrame f = new JFrame("simulation");
            f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            f.getContentPane().setBackground(BG);

            // painel central onde o mapa é desenhado
            GridPanel p = new GridPanel();
            p.setPreferredSize(new Dimension(map.getCols() * CELL + MARGIN * 2, map.getRows() * CELL + MARGIN * 2));
            p.setBackground(BG);

            // painel lateral com scroll para a lista de estados
            JPanel side = setupSidePanel();
            JScrollPane scroll = new JScrollPane(side);
            scroll.setPreferredSize(new Dimension(240, 0));
            scroll.setBorder(BorderFactory.createMatteBorder(0, 1, 0, 0, new Color(45, 45, 50)));
            scroll.getViewport().setBackground(BG);

            // barra de status no rodapé
            status = new JLabel(" ");
            status.setForeground(new Color(150, 150, 160));
            status.setBackground(BG);
            status.setOpaque(true);
            status.setBorder(BorderFactory.createEmptyBorder(10, 25, 10, 25));
            status.setFont(F_MONO);

            f.add(p, BorderLayout.CENTER);
            f.add(scroll, BorderLayout.EAST);
            f.add(status, BorderLayout.SOUTH);
            f.pack();
            f.setLocationRelativeTo(null);
            f.setVisible(true);

            // Timer para atualizar o desenho e a UI
            new Timer(16, e -> {
                p.repaint();
                updateUI();
            }).start();
        });
    }

    private JPanel setupSidePanel() {
        JPanel side = new JPanel();
        side.setLayout(new BoxLayout(side, BoxLayout.Y_AXIS));
        side.setBackground(BG);
        side.setBorder(BorderFactory.createEmptyBorder(20, 15, 20, 15));

        side.add(createTitle("LEGEND"));
        side.add(createLegendItem("Charging Station", CHARGER));
        side.add(createLegendItem("Busy Intersection", BUSY));
        side.add(createLegendItem("Low Battery (<20%)", LOW_BAT));
        side.add(createLegendItem("Diverting to Charger", DIVERT));
        side.add(Box.createVerticalStrut(20));

        side.add(createTitle("VEHICLES"));
        for (int i = 0; i < vehicles.size(); i++) {
            Vehicle v = vehicles.get(i);
            VehicleRow row = new VehicleRow(v, palette(i));
            vRows.put(v.id, row);
            side.add(row);
            side.add(Box.createVerticalStrut(10));
        }

        side.add(Box.createVerticalStrut(20));
        side.add(createTitle("CHARGING STATIONS"));
        for (ChargingStation s : stations) {
            StationRow row = new StationRow(s);
            sRows.put(s.id, row);
            side.add(row);
            side.add(Box.createVerticalStrut(10));
        }

        return side;
    }

    private JPanel createLegendItem(String text, Color color) {
        JPanel p = new JPanel(new BorderLayout(10, 0));
        p.setOpaque(false);
        p.setMaximumSize(new Dimension(210, 20));
        p.setAlignmentX(Component.LEFT_ALIGNMENT);

        JPanel dot = new JPanel() {
            @Override
            protected void paintComponent(Graphics g) {
                g.setColor(color);
                ((Graphics2D) g).setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
                g.fillOval(0, 4, 8, 8);
            }
        };
        dot.setOpaque(false);
        dot.setPreferredSize(new Dimension(12, 18));

        JLabel l = new JLabel(text);
        l.setFont(F_MAIN);
        l.setForeground(new Color(160, 160, 170));

        p.add(dot, BorderLayout.WEST);
        p.add(l, BorderLayout.CENTER);
        return p;
    }

    private JLabel createTitle(String text) {
        JLabel t = new JLabel(text);
        t.setForeground(new Color(100, 100, 115));
        t.setFont(F_BOLD);
        t.setAlignmentX(Component.LEFT_ALIGNMENT);
        return t;
    }

    private void updateUI() {
        long active = vehicles.stream().filter(v -> !v.isFinished()).count();
        status.setText(String.format("active: %d | total: %d", active, vehicles.size()));

        vRows.values().forEach(VehicleRow::update);
        sRows.values().forEach(StationRow::update);
    }

    // retorna uma cor da paleta para diferenciar veículos normais
    private Color palette(int i) {
        Color[] p = { new Color(60, 140, 255), new Color(255, 80, 80), new Color(255, 200, 30),
                new Color(170, 100, 255), new Color(40, 230, 160) };
        return p[i % p.length];
    }

    // mostra o ID, bateria (barra de progresso) e estado (a carregar, etc).
    private class VehicleRow extends JPanel {
        private final Vehicle v;
        private final JLabel label;
        private final JProgressBar bar;
        private final Color vehicleColor;

        VehicleRow(Vehicle v, Color c) {
            this.v = v;
            this.vehicleColor = c;
            setLayout(new BorderLayout(10, 2));
            setOpaque(false);
            setMaximumSize(new Dimension(210, 45));
            setAlignmentX(Component.LEFT_ALIGNMENT);

            // indicador colorido ao lado do ID
            JPanel dot = new JPanel() {
                @Override
                protected void paintComponent(Graphics g) {
                    Color corFinal;
                    if (v.isFinished()) {
                        corFinal = DONE;
                    } else if (v.getPriority() == VehiclePriority.EMERGENCY) {
                        corFinal = EMERGENCY;
                    } else {
                        corFinal = vehicleColor;
                    }

                    g.setColor(corFinal);
                    g.setColor(v.isFinished() ? DONE : vehicleColor);
                    ((Graphics2D) g).setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                            RenderingHints.VALUE_ANTIALIAS_ON);
                    g.fillOval(0, 4, 8, 8);
                }
            };
            dot.setOpaque(false);
            dot.setPreferredSize(new Dimension(12, 18));

            // define o prefixo (E para emergência, V para normal) e o texto do label
            String prefix;
            if (v.getPriority() == VehiclePriority.EMERGENCY) {
                prefix = "E";
            } else {
                prefix = "V";
            }

            label = new JLabel(prefix + v.id.substring(v.id.lastIndexOf("-") + 1));
            label.setFont(F_MAIN);

            label.setFont(F_MAIN);
            label.setForeground(new Color(200, 200, 210));

            // progresso para a bateria
            bar = new JProgressBar(0, 100);
            bar.setPreferredSize(new Dimension(100, 4));
            bar.setBackground(new Color(40, 40, 45));
            bar.setBorderPainted(false);

            JPanel top = new JPanel(new BorderLayout(5, 0));
            top.setOpaque(false);
            top.add(dot, BorderLayout.WEST);
            top.add(label, BorderLayout.CENTER);

            add(top, BorderLayout.NORTH);
            add(bar, BorderLayout.SOUTH);
        }

        void update() {
            int bat = v.getBattery();
            bar.setValue(bat);

            // cor da barra muda conforme o nível de bateria
            if (bat > 50) {
                bar.setForeground(CHARGER);
            } else if (bat > 20) {
                bar.setForeground(BUSY);
            } else {
                bar.setForeground(LOW_BAT);
            }

            String prefix;
            if (v.getPriority() == VehiclePriority.EMERGENCY) {
                prefix = "E";
            } else {
                prefix = "V";
            }

            String statusCarregamento = "";
            if (v.isCharging()) {
                statusCarregamento = " [C]";
            }

            label.setText(String.format("%s%s (%d%%)%s", prefix, v.id.substring(v.id.lastIndexOf("-") + 1), bat,
                    statusCarregamento));

            if (v.isFinished()) {
                label.setForeground(DONE);
            }
            repaint();
        }

    }

    // plugs e potencia disponiveis para cada estação de carregamento
    private class StationRow extends JPanel {
        private final ChargingStation s;
        private final JLabel info;

        StationRow(ChargingStation s) {
            this.s = s;
            setLayout(new BorderLayout());
            setOpaque(false);
            setMaximumSize(new Dimension(210, 30));
            setAlignmentX(Component.LEFT_ALIGNMENT);

            info = new JLabel();
            info.setFont(F_MAIN);
            info.setForeground(CHARGER);
            add(info, BorderLayout.CENTER);
            update();
        }

        void update() {
            info.setText(String.format("Station %d | Plugs: %d | Power: %d", s.id, s.getAvailablePlugs(),
                    s.getPowerLevel()));
        }
    }

    // desenha as estradas, interseções, carregadores e os veículos em movimento.
    private class GridPanel extends JPanel {
        // animação suave entre interseções
        private final Map<String, float[]> smoothPos = new HashMap<>();

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2 = (Graphics2D) g;
            g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            drawRoads(g2);
            drawIntersections(g2);
            drawVehicles(g2);
            drawLabels(g2);
        }

        // desenha as linhas que representam as estradas
        private void drawRoads(Graphics2D g2) {
            g2.setStroke(new BasicStroke(14f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
            g2.setColor(ROAD);
            for (int r = 0; r < map.getRows(); r++) {
                for (int c = 0; c < map.getCols(); c++) {
                    int x = MARGIN + c * CELL, y = MARGIN + r * CELL;
                    if (c < map.getCols() - 1)
                        g2.drawLine(x, y, x + CELL, y);
                    if (r < map.getRows() - 1)
                        g2.drawLine(x, y, x, y + CELL);
                }
            }
        }

        // desenha os nós das interseções e indicadores de carregador (C)
        private void drawIntersections(Graphics2D g2) {
            for (int r = 0; r < map.getRows(); r++) {
                for (int c = 0; c < map.getCols(); c++) {
                    Intersection isec = map.getIntersection(r, c);
                    if (isec == null)
                        continue;
                    int x = MARGIN + c * CELL, y = MARGIN + r * CELL;

                    boolean hasC = isec.hasChargingStation();
                    // verde se carregador, laranja se ocupado, azul se livre
                    g2.setColor(hasC ? CHARGER : (isec.getEntryCount() > 0 ? BUSY : ISEC));
                    g2.fillOval(x - 5, y - 5, 10, 10);

                    if (hasC) {
                        g2.setFont(F_BOLD);
                        g2.drawString("C" + isec.getChargingStation().id, x + 8, y - 8);
                    }
                }
            }
        }

        // desenha cada veículo como um triângulo apontando na direção do movimento
        private void drawVehicles(Graphics2D g2) {
            for (int i = 0; i < vehicles.size(); i++) {
                Vehicle v = vehicles.get(i);
                // não desenha se acabou e está sem bateria
                if (v.isFinished() && v.getBattery() <= 0)
                    continue;

                // suavização da posição para não saltar diretamente entre os blocos
                float[] p = smoothPos.computeIfAbsent(v.id,
                        k -> new float[] { (float) v.getVisualX(), (float) v.getVisualY() });
                p[0] += (v.getVisualX() - p[0]) * 0.15f;
                p[1] += (v.getVisualY() - p[1]) * 0.15f;

                int vx = MARGIN + (int) (p[0] * CELL), vy = MARGIN + (int) (p[1] * CELL);

                // cor do veículo com base no seu estado atual
                Color corVeiculo;
                if (v.isFinished()) {
                    corVeiculo = DONE;
                } else if (v.isCharging()) {
                    corVeiculo = CHARGING;
                } else if (v.getPriority() == VehiclePriority.EMERGENCY) {
                    corVeiculo = EMERGENCY;
                } else if (v.isChargingDiversion()) {
                    corVeiculo = DIVERT;
                } else if (v.getBattery() < 20) {
                    corVeiculo = LOW_BAT;
                } else {
                    corVeiculo = palette(i);
                }

                drawCar(g2, vx, vy, v.getCurrentDirection(), corVeiculo);

                // define o prefixo para o texto sobre o veículo
                String prefix;
                if (v.getPriority() == VehiclePriority.EMERGENCY) {
                    prefix = "E";
                } else {
                    prefix = "V";
                }

                String txt = prefix + v.id.substring(v.id.lastIndexOf("-") + 1) + " " + v.getBattery() + "%";
                g2.setFont(F_BOLD);
                g2.setColor(Color.WHITE);
                g2.drawString(txt, vx + 10, vy - 10);

            }
        }

        // desenho do triângulo do veículo
        private void drawCar(Graphics2D g2, int x, int y, Direction d, Color c) {
            Path2D.Float path = new Path2D.Float();
            float s = V_SIZE / 2f;
            switch (d) {
                case NORTH -> {
                    path.moveTo(x, y - s);
                    path.lineTo(x - s, y + s);
                    path.lineTo(x + s, y + s);
                }
                case SOUTH -> {
                    path.moveTo(x, y + s);
                    path.lineTo(x - s, y - s);
                    path.lineTo(x + s, y - s);
                }
                case EAST -> {
                    path.moveTo(x + s, y);
                    path.lineTo(x - s, y - s);
                    path.lineTo(x - s, y + s);
                }
                case WEST -> {
                    path.moveTo(x - s, y);
                    path.lineTo(x + s, y - s);
                    path.lineTo(x + s, y + s);
                }
            }
            path.closePath();
            g2.setColor(c);
            g2.fill(path);
            g2.setColor(Color.BLACK);
            g2.setStroke(new BasicStroke(1f));
            g2.draw(path);
        }

        // números das coordenadas nas bordas do mapa
        private void drawLabels(Graphics2D g2) {
            g2.setFont(F_MAIN);
            g2.setColor(new Color(100, 100, 110));
            for (int c = 0; c < map.getCols(); c++)
                g2.drawString(String.valueOf(c), MARGIN + c * CELL - 4, MARGIN - 20);
            for (int r = 0; r < map.getRows(); r++)
                g2.drawString(String.valueOf(r), MARGIN - 30, MARGIN + r * CELL + 5);
        }
    }
}
