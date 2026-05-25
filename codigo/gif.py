import os
import math
import random
from PIL import Image, ImageDraw, ImageFont

# Configurações de Dimensões (Proporção 16:9 ideal para slides)
LARGURA = 900
ALTURA = 500
DIRETORIO_FONTS = ["/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", "arial.ttf", "DejaVuSans.ttf"]

def obter_fonte(tamanho):
    """Tenta carregar uma fonte do sistema, se falhar usa a padrão"""
    for nome_fonte in DIRETORIO_FONTS:
        try:
            return ImageFont.truetype(nome_fonte, tamanho)
        except IOError:
            continue
    return ImageFont.load_default()

# Inicialização das fontes
fonte_titulo = obter_fonte(18)
fonte_sub = obter_fonte(13)
fonte_dados = obter_fonte(11)

def medir_texto(draw, texto, fonte):
    try:
        bbox = draw.textbbox((0, 0), texto, font=fonte)
        return bbox[2] - bbox[0], bbox[3] - bbox[1]
    except Exception:
        return draw.textsize(texto, font=fonte)

def desenhar_texto_centralizado(draw, cx, cy, texto, fonte, fill):
    largura, altura = medir_texto(draw, texto, fonte)
    draw.text((cx - largura / 2, cy - altura / 2), texto, fill=fill, font=fonte)

def desenhar_badge_status(draw, x, y, texto, cor_fundo, cor_texto=None):
    if cor_texto is None:
        cor_texto = "#1E293B"
    largura, altura = medir_texto(draw, texto, fonte_dados)
    padding_x = 10
    padding_y = 5
    caixa = [x, y, x + largura + padding_x * 2, y + altura + padding_y * 2]
    draw.rounded_rectangle(caixa, radius=10, fill=cor_fundo)
    draw.text((x + padding_x, y + padding_y), texto, fill=cor_texto, font=fonte_dados)

# Definição de Cores Estilo "Dashboard IoT Corporativo"
COR_FUNDO = "#F8FAFC"         # Off-white moderno (Slate 50)
COR_CONTEINER = "#FFFFFF"     # Fundo dos blocos
COR_SOMBRA = "#E2E8F0"        # Sombra sutil dos blocos
COR_BORDA = "#3498DB"         # Azul Primário Tech
COR_TEXTO_MAIN = "#1E293B"    # Azul Escuro Quase Preto
COR_TEXTO_MUTED = "#64748B"   # Cinza Executivo para legendas
COR_AGUA_OK = "#0EA5E9"       # Azul Oceano Vivo
COR_AGUA_CRITICA = "#EF4444"  # Vermelho Alerta Vivo
COR_SETA_INATIVA = "#CBD5E1"  # Linha de conexão em repouso
COR_SETA_ATIVA = "#22C55E"    # Verde Sucesso para pacotes ativos

def desenhar_base_arquitetura(draw, titulo_passo, status_agua="normal", nivel_cm=92.0, 
                             msg_middleware=None, msg_bot=None, 
                             msg_notif="Status: Standby", cor_badge_notif=None):
    """Desenha a infraestrutura de blocos com cantos arredondados, sombras e estados dinâmicos limpos"""
    # Fundo Geral
    draw.rectangle([0, 0, LARGURA, ALTURA], fill=COR_FUNDO)
    
    # Barra de Cabeçalho Fixa
    draw.rectangle([0, 0, LARGURA, 60], fill="#1E3A8A")
    draw.text((25, 18), "SISTEMA DISTRIBUIDO DE MONITORAMENTO HIDRICO (UFMA 2026)", fill="#FFFFFF", font=fonte_titulo)
    
    # Subtítulo com o Passo Atual da Animação
    draw.text((25, 75), f"Status do Pipeline: {titulo_passo}", fill=COR_TEXTO_MUTED, font=fonte_sub)
    
    # --- BLOCO 1: RESERVATÓRIO & ESP32 ---
    draw.rounded_rectangle([40, 125, 200, 435], radius=8, fill=COR_SOMBRA)
    draw.rounded_rectangle([40, 120, 200, 430], radius=8, fill=COR_CONTEINER, outline=COR_BORDA, width=2)
    draw.text((55, 135), "1. AQUISICAO\n(ESP32 Simulador)", fill=COR_TEXTO_MAIN, font=fonte_sub)
    
    # Tanque Físico de Água
    cor_atual_agua = COR_AGUA_OK if status_agua == "normal" else COR_AGUA_CRITICA
    altura_agua = 240 if status_agua == "normal" else 375 
    
    # Preenchimento interno do tanque
    draw.rectangle([65, altura_agua, 175, 390], fill=cor_atual_agua)
    # Borda do Tanque
    draw.rectangle([65, 210, 175, 390], outline=COR_TEXTO_MUTED, width=2)
    draw.text((75, 402), f"Nivel: {nivel_cm:.1f} cm", fill=COR_TEXTO_MAIN, font=fonte_dados)

    # --- BLOCO 2: MIDDLEWARE MOCK (Server Mentira) ---
    draw.rounded_rectangle([290, 125, 510, 245], radius=8, fill=COR_SOMBRA)
    draw.rounded_rectangle([290, 120, 510, 240], radius=8, fill=COR_CONTEINER, outline=COR_BORDA, width=2)
    draw.text((305, 135), "2. INTEGRACAO\n(Flask Mock Server)", fill=COR_TEXTO_MAIN, font=fonte_sub)
    draw.text((305, 185), "POST /collector/data\nGET  /data/last", fill=COR_BORDA, font=fonte_dados)
    
    if msg_middleware:
        draw.rounded_rectangle([290, 120, 510, 240], radius=8, fill=None, outline=COR_SETA_ATIVA, width=3)
        largura_m, _ = medir_texto(draw, msg_middleware, fonte_dados)
        draw.text((400 - largura_m / 2, 215), msg_middleware, fill=COR_SETA_ATIVA, font=fonte_dados)

    # --- BLOCO 3: BOT MONITOR EM C ---
    draw.rounded_rectangle([290, 315, 510, 435], radius=8, fill=COR_SOMBRA)
    draw.rounded_rectangle([290, 310, 510, 430], radius=8, fill=COR_CONTEINER, outline=COR_BORDA, width=2)
    draw.text((305, 325), "3. PROCESSAMENTO\n(Bot C Monitor)", fill=COR_TEXTO_MAIN, font=fonte_sub)
    draw.text((305, 375), "Loop de Auditoria\nLimiar: < 15.0 cm", fill=COR_TEXTO_MUTED, font=fonte_dados)
    
    if msg_bot:
        cor_bot_msg = COR_AGUA_CRITICA if status_agua == "critico" else COR_SETA_ATIVA
        draw.rounded_rectangle([290, 310, 510, 430], radius=8, fill=None, outline=cor_bot_msg, width=3)
        largura_b, _ = medir_texto(draw, msg_bot, fonte_dados)
        draw.text((400 - largura_b / 2, 405), msg_bot, fill=cor_bot_msg, font=fonte_dados)

    # --- BLOCO 4: NOTIFICAÇÃO (Evolution API) ---
    if cor_badge_notif is None:
        cor_badge_notif = COR_SETA_INATIVA
        
    cor_borda_notif = COR_SETA_ATIVA if cor_badge_notif == COR_SETA_ATIVA else COR_BORDA
    width_borda_notif = 3 if cor_badge_notif == COR_SETA_ATIVA else 2
    
    draw.rounded_rectangle([600, 215, 840, 335], radius=8, fill=COR_SOMBRA)
    draw.rounded_rectangle([600, 210, 840, 330], radius=8, fill=COR_CONTEINER, outline=cor_borda_notif, width=width_borda_notif)
    draw.text((615, 225), "4. NOTIFICACAO\n(Evolution API + Docker)", fill=COR_TEXTO_MAIN, font=fonte_sub)
    
    cor_txt_badge = "#FFFFFF" if cor_badge_notif == COR_SETA_ATIVA else "#1E293B"
    desenhar_badge_status(draw, 615, 270, msg_notif, cor_badge_notif, cor_txt_badge)

def desenhar_seta(draw, x1, y1, x2, y2, ativa=False, rotulo=""):
    """Desenha conexões inteligentes com posicionamento de texto anti-sobreposição"""
    cor = COR_SETA_ATIVA if ativa else COR_SETA_INATIVA
    largura = 3 if ativa else 1
    
    # Desenha a linha da conexão
    draw.line([x1, y1, x2, y2], fill=cor, width=largura)
    
    # Ponta da Seta Dinâmica baseado na direção do vetor
    if x1 < x2 and y1 == y2: # Direita pura
        draw.polygon([x2, y2, x2-10, y2-6, x2-10, y2+6], fill=cor)
    elif x1 == x2 and y1 < y2: # Baixo pura
        draw.polygon([x2, y2, x2-6, y2-10, x2+6, y2-10], fill=cor)
    elif x1 == x2 and y1 > y2: # Cima pura
        draw.polygon([x2, y2, x2-6, y2+10, x2+6, y2+10], fill=cor)
    else: # Diagonal ou genérica
        draw.polygon([x2, y2, x2-8, y2-4, x2-4, y2-8], fill=cor)
        
    # Tratamento de rótulos para evitar colisões com os blocos
    if rotulo and ativa:
        meio_x = (x1 + x2) / 2
        meio_y = (y1 + y2) / 2
        largura_txt, altura_txt = medir_texto(draw, rotulo, fonte_dados)

        if abs(y2 - y1) <= 2: # Linhas Horizontais
            coords_texto = (meio_x - largura_txt / 2, min(y1, y2) - altura_txt - 8)
        elif abs(x2 - x1) <= 2: # Linhas Verticais
            if y2 >= y1:
                coords_texto = (max(x1, x2) + 10, meio_y - altura_txt / 2)
            else:
                coords_texto = (min(x1, x2) - largura_txt - 10, meio_y - altura_txt / 2)
        else: # Linhas Diagonais
            coords_texto = (meio_x - largura_txt - 8, meio_y - altura_txt / 2)

        draw.text(coords_texto, rotulo, fill=COR_SETA_ATIVA, font=fonte_dados)

def criar_animacao_sistema():
    frames = []

    # Configuração estável dos pontos de conexão da topologia de rede
    OUT_ESP32 = 200; IN_MOCK_X = 290; Y_LINHA_POST = 180
    MID_MOCK_X = 400; OUT_MOCK_Y = 240; IN_BOT_Y = 310
    OUT_BOT_X = 510; IN_EVO_X = 600; Y_LINHA_TRIGGER = 370; Y_EVO_TARGET = 270
    OUT_EVO_X_FINAL = 840; END_WAR_X = 895; Y_FINAL_WA = 270

    # QUADRO 1: Estado Inicial Estável
    img1 = Image.new("RGB", (LARGURA, ALTURA))
    draw1 = ImageDraw.Draw(img1)
    desenhar_base_arquitetura(draw1, "Aguardando telemetria do sensor...", "normal", 92.4)
    desenhar_seta(draw1, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=False)
    desenhar_seta(draw1, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=False)
    desenhar_seta(draw1, OUT_BOT_X, Y_LINHA_TRIGGER, IN_EVO_X, Y_EVO_TARGET, ativa=False)
    desenhar_seta(draw1, OUT_EVO_X_FINAL, Y_FINAL_WA, END_WAR_X, Y_FINAL_WA, ativa=False)
    frames.append(img1)

    # QUADRO 2: ESP32 Despacha Carga Útil (POST JSON)
    img2 = Image.new("RGB", (LARGURA, ALTURA))
    draw2 = ImageDraw.Draw(img2)
    desenhar_base_arquitetura(draw2, "ESP32 serializa dados senoidais e transmite via HTTP POST", "normal", 92.4)
    desenhar_seta(draw2, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=True, rotulo="POST (JSON)")
    desenhar_seta(draw2, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=False)
    desenhar_seta(draw2, OUT_BOT_X, Y_LINHA_TRIGGER, IN_EVO_X, Y_EVO_TARGET, ativa=False)
    frames.append(img2)

    # QUADRO 3: Servidor Armazena e Confirma (HTTP 200)
    img3 = Image.new("RGB", (LARGURA, ALTURA))
    draw3 = ImageDraw.Draw(img3)
    desenhar_base_arquitetura(draw3, "Mock Server atualiza o cache volatil em memoria (Thread-safe)", "normal", 94.1, msg_middleware="Cache Atualizado")
    desenhar_seta(draw3, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=False)
    desenhar_seta(draw3, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=False)
    frames.append(img3)

    # QUADRO 4: Auditoria em Execução (Bot C faz Polling)
    img4 = Image.new("RGB", (LARGURA, ALTURA))
    draw4 = ImageDraw.Draw(img4)
    desenhar_base_arquitetura(draw4, "Bot Monitor em C realiza chamada GET periodica buscando o ultimo dado", "normal", 94.1)
    desenhar_seta(draw4, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=False)
    desenhar_seta(draw4, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=True, rotulo="GET /data/last")
    desenhar_seta(draw4, OUT_BOT_X, Y_LINHA_TRIGGER, IN_EVO_X, Y_EVO_TARGET, ativa=False)
    frames.append(img4)

    # QUADRO 5: Transição Crítica e Detecção Automática
    img5 = Image.new("RGB", (LARGURA, ALTURA))
    draw5 = ImageDraw.Draw(img5)
    desenhar_base_arquitetura(draw5, "Nivel cai drasticamente no reservatorio. Bot identifica violacao do limiar", "critico", 12.3, msg_bot="ALERTA: NIVEL CRITICO!")
    desenhar_seta(draw5, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=False)
    desenhar_seta(draw5, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=False)
    desenhar_seta(draw5, OUT_BOT_X, Y_LINHA_TRIGGER, IN_EVO_X, Y_EVO_TARGET, ativa=False)
    frames.append(img5)

    # QUADRO 6: Ativação da Camada de Notificação (Trigger Bot)
    img6 = Image.new("RGB", (LARGURA, ALTURA))
    draw6 = ImageDraw.Draw(img6)
    desenhar_base_arquitetura(draw6, "Bot em C dispara requisicao de contingencia para o container de mensageria", "critico", 12.3, msg_bot="ALERTA: NIVEL CRITICO!")
    desenhar_seta(draw6, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=False)
    desenhar_seta(draw6, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=False)
    desenhar_seta(draw6, OUT_BOT_X, Y_LINHA_TRIGGER, IN_EVO_X, Y_EVO_TARGET, ativa=True, rotulo="Trigger Bot")
    desenhar_seta(draw6, OUT_EVO_X_FINAL, Y_FINAL_WA, END_WAR_X, Y_FINAL_WA, ativa=False)
    frames.append(img6)

    # QUADRO 7: Disparo Exclusivo para o Canal WhatsApp
    img7 = Image.new("RGB", (LARGURA, ALTURA))
    draw7 = ImageDraw.Draw(img7)
    desenhar_base_arquitetura(draw7, "Evolution API processa a fila e envia notificacao Push para o WhatsApp do gestor", "critico", 12.3, msg_bot="ALERTA: NIVEL CRITICO!", msg_notif="Status: ENVIANDO ALERTA...", cor_badge_notif=COR_SETA_ATIVA)
    desenhar_seta(draw7, OUT_ESP32, Y_LINHA_POST, IN_MOCK_X, Y_LINHA_POST, ativa=False)
    desenhar_seta(draw7, MID_MOCK_X, OUT_MOCK_Y, MID_MOCK_X, IN_BOT_Y, ativa=False)
    desenhar_seta(draw7, OUT_BOT_X, Y_LINHA_TRIGGER, IN_EVO_X, Y_EVO_TARGET, ativa=False)
    desenhar_seta(draw7, OUT_EVO_X_FINAL, Y_FINAL_WA, END_WAR_X, Y_FINAL_WA, ativa=True, rotulo="WhatsApp")
    frames.append(img7)

    # Geração física do arquivo GIF com compressão otimizada
    nome_arquivo = "funcionamento_sistema.gif"
    frames[0].save(
        nome_arquivo,
        save_all=True,
        append_images=frames[1:],
        optimize=True,
        duration=2000,  # 2 segundos por frame para leitura confortável na banca
        loop=0
    )
    print(f"Sucesso absoluto! Arquivo '{nome_arquivo}' gerado com design limpo.")

if __name__ == "__main__":
    criar_animacao_sistema()