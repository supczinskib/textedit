//
// "$Id: editor.cxx 5519 2006-10-11 03:12:15Z mike $"
//
// A simple text editor program for the Fast Light Tool Kit (FLTK).
//
// This program is described in Chapter 4 of the FLTK Programmer's Guide.
//
// Copyright 1998-2005 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// -------------------------- Change Log -------------------------- //
// Ver 0.2 1/21/2025
// Changed by: Rich for Tinycore Linux.
// Removed syntax highlighting.
// Disabled "drag and drop text". You can re-enable it with  ~/.Xdefaults:
//        fltk*dndTextOps: true
// Added version number to source.
// Added Version menu entry.
// Added line number and line wrap code lifted from a later version of this program.
// I added these options in preprocessor directives:
//     Uncomment #define DIS_DNDTEXT to disable drag and drop text.
//     Uncomment #define ENA_LINEWRAP to enable line wrapping in Edit menu.
//     Uncomment #define ENA_LINENUMS to enable line numbers in Edit menu.
//
// --------------------------- End Log ---------------------------- //

#define APP_FONT FL_HELVETICA
#define APP_FONT_SIZE 11
#define Editor_TextFont APP_FONT
#define Editor_TextSize APP_FONT_SIZE
#define Editor_CJKFont ((Fl_Font)(FL_FREE_FONT + 1))


#define Version "Ver 0.8 4/24/2026"

// Uncomment the next line to disable drag and drop text.
#define DIS_DNDTEXT

// Uncomment the next line to enable linewrap option in Edit menu.
#define ENA_LINEWRAP

// Uncomment the next line to enable linenumber option in Edit menu.
#define ENA_LINENUMS

#ifdef ENA_LINENUMS
// Width of line number display, if enabled.
const int line_num_width = 40;
#endif // ENA_LINENUMS

// Target device visible work area: 480x320 screen with 24 px used by the system bar.
// Leave additional room for the window-manager titlebar/decorations so the
// Fl_File_Chooser titlebar stays visible on the LCD.
#define APP_FILE_CHOOSER_MAX_W 480
#define APP_FILE_CHOOSER_WORK_H 296
#define APP_FILE_CHOOSER_TITLEBAR_ROOM 30
#define APP_FILE_CHOOSER_MAX_H (APP_FILE_CHOOSER_WORK_H - APP_FILE_CHOOSER_TITLEBAR_ROOM)

//
// Include necessary headers...
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <string>

#ifdef __MWERKS__
# define FL_DLL
#endif

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Browser_.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>

extern const char *fl_ok;
extern const char *fl_cancel;
extern const char *fl_yes;
extern const char *fl_no;
extern const char *fl_close;


struct TranslationEntry {
  const char *key;
  const char *ar;
  const char *zh_CN;
  const char *nl;
  const char *en;
  const char *fr;
  const char *de;
  const char *it;
  const char *pl;
  const char *pt;
  const char *ru;
  const char *es;
  const char *tr;
};

static const char *app_lang = "en";

static int lang_matches(const char *value, const char *lang_code) {
  if (!value || !*value || !lang_code || !*lang_code) return 0;
  size_t n = strlen(lang_code);
  return strncmp(value, lang_code, n) == 0 &&
         (value[n] == '\0' || value[n] == '_' || value[n] == '-' || value[n] == '.');
}

static const char *detect_app_language() {
  const char *value = getenv("LANGUAGE");

  if (lang_matches(value, "ar")) return "ar";
  if (lang_matches(value, "zh_CN") || lang_matches(value, "zh")) return "zh_CN";
  if (lang_matches(value, "nl")) return "nl";
  if (lang_matches(value, "fr")) return "fr";
  if (lang_matches(value, "de")) return "de";
  if (lang_matches(value, "it")) return "it";
  if (lang_matches(value, "pl")) return "pl";
  if (lang_matches(value, "pt")) return "pt";
  if (lang_matches(value, "ru")) return "ru";
  if (lang_matches(value, "es")) return "es";
  if (lang_matches(value, "tr")) return "tr";
  if (lang_matches(value, "en")) return "en";

  return "en";
}

static const TranslationEntry translations[] = {
  {"Replace", "استبدال", "替换", "Vervangen", "Replace", "Remplacer", "Ersetzen", "Sostituisci", "Zamień", "Substituir", "Заменить", "Reemplazar", "Değiştir"},
  {"Find:", "بحث:", "查找:", "Zoeken:", "Find:", "Rechercher :", "Suchen:", "Trova:", "Szukaj:", "Localizar:", "Найти:", "Buscar:", "Bul:"},
  {"Replace:", "استبدال بـ:", "替换为:", "Vervangen door:", "Replace:", "Remplacer par :", "Ersetzen durch:", "Sostituisci con:", "Zamień na:", "Substituir por:", "Заменить на:", "Reemplazar por:", "Şununla değiştir:"},
  {"Replace All", "استبدال الكل", "全部替换", "Alles vervangen", "Replace All", "Tout remplacer", "Alle ersetzen", "Sostituisci tutto", "Zamień wszystko", "Substituir tudo", "Заменить все", "Reemplazar todo", "Tümünü değiştir"},
  {"Replace Next", "استبدال التالي", "替换下一个", "Volgende vervangen", "Replace Next", "Remplacer le suivant", "Nächstes ersetzen", "Sostituisci successivo", "Zamień następne", "Substituir próximo", "Заменить следующее", "Reemplazar siguiente", "Sonrakini değiştir"},
  {"OK", "موافق", "确定", "OK", "OK", "OK", "OK", "OK", "OK", "OK", "ОК", "Aceptar", "Tamam"},
  {"Cancel", "إلغاء", "取消", "Annuleren", "Cancel", "Annuler", "Abbrechen", "Annulla", "Anuluj", "Cancelar", "Отмена", "Cancelar", "İptal"},
  {"Yes", "نعم", "是", "Ja", "Yes", "Oui", "Ja", "Sì", "Tak", "Sim", "Да", "Sí", "Evet"},
  {"No", "لا", "否", "Nee", "No", "Non", "Nein", "No", "Nie", "Não", "Нет", "No", "Hayır"},
  {"Close", "إغلاق", "关闭", "Sluiten", "Close", "Fermer", "Schließen", "Chiudi", "Zamknij", "Fechar", "Закрыть", "Cerrar", "Kapat"},
  {"Preview", "معاينة", "预览", "Voorbeeld", "Preview", "Aperçu", "Vorschau", "Anteprima", "Podgląd", "Pré-visualizar", "Предпросмотр", "Vista previa", "Önizleme"},
  {"New Directory", "مجلد جديد", "新建目录", "Nieuwe map", "New Directory", "Nouveau dossier", "Neues Verzeichnis", "Nuova cartella", "Nowy katalog", "Novo diretório", "Новый каталог", "Nuevo directorio", "Yeni dizin"},
  {"New Directory?", "دليل جديد؟", "新建目录？", "Nieuwe map?", "New Directory?", "Nouveau dossier ?", "Neues Verzeichnis?", "Nuova cartella?", "Nowy katalog?", "Novo diretório?", "Новый каталог?", "¿Nuevo directorio?", "Yeni dizin?"},
  {"Create a new directory.", "إنشاء دليل جديد.", "创建新目录。", "Een nieuwe map maken.", "Create a new directory.", "Créer un nouveau dossier.", "Neues Verzeichnis erstellen.", "Crea una nuova cartella.", "Utwórz nowy katalog.", "Criar um novo diretório.", "Создать новый каталог.", "Crear un nuevo directorio.", "Yeni bir dizin oluştur."},
  {"Show hidden files", "إظهار الملفات المخفية", "显示隐藏文件", "Verborgen bestanden tonen", "Show hidden files", "Afficher les fichiers cachés", "Versteckte Dateien anzeigen", "Mostra file nascosti", "Pokaż ukryte pliki", "Mostrar ficheiros ocultos", "Показывать скрытые файлы", "Mostrar archivos ocultos", "Gizli dosyaları göster"},
  {"Show:", "عرض:", "显示:", "Weergeven:", "Show:", "Afficher :", "Anzeigen:", "Mostra:", "Pokaż:", "Mostrar:", "Показать:", "Mostrar:", "Göster:"},
  {"Filename:", "اسم الملف:", "文件名:", "Bestandsnaam:", "Filename:", "Nom du fichier :", "Dateiname:", "Nome file:", "Nazwa pliku:", "Nome do ficheiro:", "Имя файла:", "Nombre del archivo:", "Dosya adı:"},
  {"All Files (*)", "كل الملفات (*)", "所有文件 (*)", "Alle bestanden (*)", "All Files (*)", "Tous les fichiers (*)", "Alle Dateien (*)", "Tutti i file (*)", "Wszystkie pliki (*)", "Todos os ficheiros (*)", "Все файлы (*)", "Todos los archivos (*)", "Tüm dosyalar (*)"},
  {"Custom Filter", "مرشح مخصص", "自定义筛选器", "Aangepast filter", "Custom Filter", "Filtre personnalisé", "Benutzerdefinierter Filter", "Filtro personalizzato", "Filtr niestandardowy", "Filtro personalizado", "Пользовательский фильтр", "Filtro personalizado", "Özel filtre"},
  {"Favorites", "المفضلة", "收藏夹", "Favorieten", "Favorites", "Favoris", "Favoriten", "Preferiti", "Ulubione", "Favoritos", "Избранное", "Favoritos", "Favoriler"},
  {"Add to Favorites", "أضف إلى المفضلة", "添加到收藏夹", "Toevoegen aan favorieten", "Add to Favorites", "Ajouter aux favoris", "Zu Favoriten hinzufügen", "Aggiungi ai preferiti", "Dodaj do ulubionych", "Adicionar aos favoritos", "Добавить в избранное", "Agregar a favoritos", "Favorilere ekle"},
  {"Manage Favorites", "إدارة المفضلة", "管理收藏夹", "Favorieten beheren", "Manage Favorites", "Gérer les favoris", "Favoriten verwalten", "Gestisci preferiti", "Zarządzaj ulubionymi", "Gerir favoritos", "Управление избранным", "Administrar favoritos", "Favorileri yönet"},
  {"File Systems", "أنظمة الملفات", "文件系统", "Bestandssystemen", "File Systems", "Systèmes de fichiers", "Dateisysteme", "File system", "Systemy plików", "Sistemas de ficheiros", "Файловые системы", "Sistemas de archivos", "Dosya sistemleri"},
  {"Please choose an existing file!", "يرجى اختيار ملف موجود!", "请选择一个现有文件！", "Kies een bestaand bestand!", "Please choose an existing file!", "Veuillez choisir un fichier existant !", "Bitte wählen Sie eine vorhandene Datei aus!", "Seleziona un file esistente!", "Wybierz istniejący plik!", "Escolha um ficheiro existente!", "Выберите существующий файл!", "¡Elija un archivo existente!", "Lütfen mevcut bir dosya seçin!"},
  {"The current file has not been saved.\nWould you like to save it now?", "Bieżący plik nie został zapisany.\nCzy chcesz go teraz zapisać?", "当前文件尚未保存。\n是否现在保存？", "Het huidige bestand is niet opgeslagen.\nWilt u het nu opslaan?", "The current file has not been saved.\nWould you like to save it now?", "Le fichier actuel n'a pas été enregistré.\nVoulez-vous l'enregistrer maintenant ?", "Die aktuelle Datei wurde nicht gespeichert.\nMöchten Sie sie jetzt speichern?", "Il file corrente non è stato salvato.\nVuoi salvarlo ora?", "Bieżący plik nie został zapisany.\nCzy chcesz go teraz zapisać?", "O ficheiro atual não foi guardado.\nDeseja guardá-lo agora?", "Текущий файл не был сохранён.\nСохранить его сейчас?", "El archivo actual no se ha guardado.\n¿Desea guardarlo ahora?", "Geçerli dosya kaydedilmedi.\nŞimdi kaydetmek istiyor musunuz?"},
  {"Save", "حفظ", "保存", "Opslaan", "Save", "Enregistrer", "Speichern", "Salva", "Zapisz", "Guardar", "Сохранить", "Guardar", "Kaydet"},
  {"Don't Save", "لا تحفظ", "不保存", "Niet opslaan", "Don't Save", "Ne pas enregistrer", "Nicht speichern", "Non salvare", "Nie zapisuj", "Não guardar", "Не сохранять", "No guardar", "Kaydetme"},
  {"Error reading from file '%s':\n%s.", "Błąd odczytu pliku '%s':\n%s.", "读取文件 '%s' 时出错：\n%s。", "Fout bij lezen van bestand '%s':\n%s.", "Error reading from file '%s':\n%s.", "Erreur de lecture du fichier '%s' :\n%s.", "Fehler beim Lesen der Datei '%s':\n%s.", "Errore durante la lettura del file '%s':\n%s.", "Błąd odczytu pliku '%s':\n%s.", "Erro ao ler o ficheiro '%s':\n%s.", "Ошибка чтения файла '%s':\n%s.", "Error al leer el archivo '%s':\n%s.", "'%s' dosyası okunurken hata oluştu:\n%s."},
  {"Error writing to file '%s':\n%s.", "Błąd zapisu pliku '%s':\n%s.", "写入文件 '%s' 时出错：\n%s。", "Fout bij schrijven naar bestand '%s':\n%s.", "Error writing to file '%s':\n%s.", "Erreur d'écriture dans le fichier '%s' :\n%s.", "Fehler beim Schreiben der Datei '%s':\n%s.", "Errore durante la scrittura del file '%s':\n%s.", "Błąd zapisu pliku '%s':\n%s.", "Erro ao escrever no ficheiro '%s':\n%s.", "Ошибка записи файла '%s':\n%s.", "Error al escribir en el archivo '%s':\n%s.", "'%s' dosyasına yazılırken hata oluştu:\n%s."},
  {"Search String:", "نص البحث:", "查找内容:", "Zoektekst:", "Search String:", "Texte à rechercher :", "Suchtext:", "Testo da cercare:", "Szukany tekst:", "Texto a procurar:", "Строка поиска:", "Texto a buscar:", "Aranacak metin:"},
  {"No occurrences of '%s' found!", "Nie znaleziono wystąpień '%s'!", "未找到 '%s' 的任何匹配项！", "Geen voorkomens van '%s' gevonden!", "No occurrences of '%s' found!", "Aucune occurrence de '%s' n'a été trouvée !", "Keine Vorkommen von '%s' gefunden!", "Nessuna occorrenza di '%s' trovata!", "Nie znaleziono wystąpień '%s'!", "Nenhuma ocorrência de '%s' encontrada!", "Вхождения '%s' не найдены!", "No se encontraron coincidencias de '%s'.", "'%s' için hiç eşleşme bulunamadı!"},
  {"Untitled", "بدون عنوان", "未命名", "Naamloos", "Untitled", "Sans titre", "Unbenannt", "Senza titolo", "Bez tytułu", "Sem título", "Без имени", "Sin título", "Adsız"},
  {" (modified)", " (zmodyfikowany)", "（已修改）", " (gewijzigd)", " (modified)", " (modifié)", " (geändert)", " (modificato)", " (zmodyfikowany)", " (modificado)", " (изменён)", " (modificado)", " (değiştirildi)"},
  {"Open File?", "Otworzyć plik?", "打开文件？", "Bestand openen?", "Open File?", "Ouvrir un fichier ?", "Datei öffnen?", "Aprire il file?", "Otworzyć plik?", "Abrir ficheiro?", "Открыть файл?", "¿Abrir archivo?", "Dosya açılsın mı?"},
  {"Insert File?", "Wstawić plik?", "插入文件？", "Bestand invoegen?", "Insert File?", "Insérer un fichier ?", "Datei einfügen?", "Inserire il file?", "Wstawić plik?", "Inserir ficheiro?", "Вставить файл?", "¿Insertar archivo?", "Dosya eklensin mi?"},
  {"Replaced %d occurrences.", "Zamieniono %d wystąpień.", "已替换 %d 处。", "%d voorkomens vervangen.", "Replaced %d occurrences.", "%d occurrences remplacées.", "%d Vorkommen ersetzt.", "Sostituite %d occorrenze.", "Zamieniono %d wystąpień.", "%d ocorrências substituídas.", "Заменено %d вхождений.", "Se reemplazaron %d coincidencias.", "%d eşleşme değiştirildi."},
  {"Save File As?", "Zapisać plik jako?", "另存为？", "Bestand opslaan als?", "Save File As?", "Enregistrer le fichier sous ?", "Datei speichern unter?", "Salvare il file come?", "Zapisać plik jako?", "Guardar ficheiro como?", "Сохранить файл как?", "¿Guardar archivo como?", "Dosya farklı kaydedilsin mi?"},
  {"&File", "&Plik", "文件(&F)", "&Bestand", "&File", "&Fichier", "&Datei", "&File", "&Plik", "&Ficheiro", "&Файл", "&Archivo", "&Dosya"},
  {"&New File", "&Nowy plik", "新建文件(&N)", "&Nieuw bestand", "&New File", "&Nouveau fichier", "&Neue Datei", "&Nuovo file", "&Nowy plik", "&Novo ficheiro", "&Новый файл", "&Nuevo archivo", "&Yeni dosya"},
  {"&Open File...", "&Otwórz plik...", "打开文件(&O)...", "Bestand &openen...", "&Open File...", "&Ouvrir un fichier...", "Datei ö&ffnen...", "&Apri file...", "&Otwórz plik...", "&Abrir ficheiro...", "&Открыть файл...", "&Abrir archivo...", "&Dosya aç..."},
  {"&Insert File...", "&Wstaw plik...", "插入文件(&I)...", "Bestand &invoegen...", "&Insert File...", "&Insérer un fichier...", "Datei &einfügen...", "&Inserisci file...", "&Wstaw plik...", "&Inserir ficheiro...", "Вст&авить файл...", "&Insertar archivo...", "&Dosya ekle..."},
  {"&Save File", "&Zapisz plik", "保存文件(&S)", "Bestand op&slaan", "&Save File", "&Enregistrer le fichier", "Datei &speichern", "&Salva file", "&Zapisz plik", "&Guardar ficheiro", "&Сохранить файл", "&Guardar archivo", "&Dosyayı kaydet"},
  {"Save File &As...", "Zapisz plik &jako...", "文件另存为(&A)...", "Bestand opslaan &als...", "Save File &As...", "Enregistrer le fichier &sous...", "Datei speichern &unter...", "Salva file &come...", "Zapisz plik &jako...", "Guardar ficheiro &como...", "Сохранить файл &как...", "Guardar archivo &como...", "Dosyayı farklı &kaydet..."},
  {"New &View", "Nowy &widok", "新建视图(&V)", "Nieuwe &weergave", "New &View", "Nouvelle &vue", "Neue &Ansicht", "Nuova &vista", "Nowy &widok", "Nova &vista", "Новое пре&дставление", "Nueva &vista", "Yeni &görünüm"},
  {"&Close View", "Za&mknij widok", "关闭视图(&C)", "Weergave s&luiten", "&Close View", "&Fermer la vue", "Ansicht &schließen", "&Chiudi vista", "Za&mknij widok", "&Fechar vista", "&Закрыть вид", "&Cerrar vista", "Görünümü &kapat"},
  {"E&xit", "&Zakończ", "退出(&X)", "A&fsluiten", "E&xit", "&Quitter", "B&eenden", "&Esci", "&Zakończ", "&Sair", "&Выход", "&Salir", "Çı&kış"},
  {"&Edit", "&Edycja", "编辑(&E)", "Be&werken", "&Edit", "&Édition", "&Bearbeiten", "&Modifica", "&Edycja", "&Editar", "&Правка", "&Editar", "Dü&zen"},
  {"Cu&t", "Wy&tnij", "剪切(&T)", "Kni&ppen", "Cu&t", "Cou&per", "Aussch&neiden", "Ta&glia", "Wy&tnij", "Cor&tar", "Выре&зать", "Cor&tar", "K&es"},
  {"&Copy", "&Kopiuj", "复制(&C)", "&Kopiëren", "&Copy", "Co&pier", "&Kopieren", "&Copia", "&Kopiuj", "&Copiar", "&Копировать", "&Copiar", "K&opyala"},
  {"&Paste", "Wk&lej", "粘贴(&P)", "P&lakken", "&Paste", "Co&ller", "&Einfügen", "Incoll&a", "Wk&lej", "Co&lar", "Вст&авить", "&Pegar", "Ya&pıştır"},
  {"&Delete", "&Usuń", "删除(&D)", "Verwij&deren", "&Delete", "&Supprimer", "&Löschen", "&Elimina", "&Usuń", "&Eliminar", "&Удалить", "&Eliminar", "&Sil"},
  {"Line &Numbers ", "Numery &wierszy", "行号(&N)", "Regel&nummers", "Line &Numbers ", "Numéros de &ligne", "Zeilen&nummern", "Numeri di &riga", "Numery &wierszy", "Números de &linha", "Номера &строк", "Números de &línea", "Satır &numaraları"},
  {"Word Wrap", "Zawijanie wierszy", "自动换行", "Regelafbreking", "Word Wrap", "Retour automatique à la ligne", "Zeilenumbruch", "A capo automatico", "Zawijanie wierszy", "Quebra automática de linha", "Перенос строк", "Ajuste de línea", "Satır kaydırma"},
  {"&Search", "&Szukaj", "搜索(&S)", "&Zoeken", "&Search", "&Recherche", "&Suchen", "&Cerca", "&Szukaj", "&Pesquisar", "&Поиск", "&Buscar", "&Ara"},
  {"&Find...", "&Znajdź...", "查找(&F)...", "&Zoeken...", "&Find...", "&Rechercher...", "&Suchen...", "&Trova...", "&Znajdź...", "&Localizar...", "&Найти...", "&Buscar...", "&Bul..."},
  {"F&ind Again", "Znajdź &ponownie", "再次查找(&I)", "Opnieuw v&inden", "F&ind Again", "Rechercher de nouveau", "Erneut su&chen", "Trova anc&ora", "Znajdź &ponownie", "Localizar novam&ente", "Найти &снова", "Buscar de n&uevo", "Tekrar bu&l"},
  {"&Replace...", "&Zamień...", "替换(&R)...", "Ve&rvangen...", "&Replace...", "Rem&placer...", "&Ersetzen...", "&Sostituisci...", "&Zamień...", "&Substituir...", "&Заменить...", "&Reemplazar...", "D&eğiştir..."},
  {"Re&place Again", "Zamień p&onownie", "再次替换(&P)", "Opnieuw ver&vangen", "Re&place Again", "Re&mplacer à nouveau", "Erneut erset&zen", "Sostituisci anc&ora", "Zamień p&onownie", "Substituir novam&ente", "Заменить с&нова", "Reemplazar de n&uevo", "Tekrar d&eğiştir"},
  {"&Version", "&Wersja", "版本(&V)", "&Versie", "&Version", "&Version", "&Version", "&Versione", "&Wersja", "&Versão", "&Версия", "&Versión", "&Sürüm"}
};

static const char *tr(const char *key) {
  for (size_t i = 0; i < sizeof(translations) / sizeof(translations[0]); ++i) {
    if (strcmp(translations[i].key, key) == 0) {
      if (strcmp(app_lang, "ar") == 0) return translations[i].ar;
      if (strcmp(app_lang, "zh_CN") == 0) return translations[i].zh_CN;
      if (strcmp(app_lang, "nl") == 0) return translations[i].nl;
      if (strcmp(app_lang, "fr") == 0) return translations[i].fr;
      if (strcmp(app_lang, "de") == 0) return translations[i].de;
      if (strcmp(app_lang, "it") == 0) return translations[i].it;
      if (strcmp(app_lang, "pl") == 0) return translations[i].pl;
      if (strcmp(app_lang, "pt") == 0) return translations[i].pt;
      if (strcmp(app_lang, "ru") == 0) return translations[i].ru;
      if (strcmp(app_lang, "es") == 0) return translations[i].es;
      if (strcmp(app_lang, "tr") == 0) return translations[i].tr;
      return translations[i].en;
    }
  }

  return key;
}



// -------------------- Application color resources -------------------- //
// Optional system-wide color file:
//   /usr/share/X11/app-defaults/TextEdit
//
// Supported keys, in X resources style:
//   TextEdit.dialogBackground:    #2f2e2b    // general window/dialog background
//   TextEdit.fieldBackground:         #3c3b37    // text/list/input background
//   TextEdit.foreground:          #ffffff
//   TextEdit.selection:           #6AA835
//   TextEdit.textBackground:      #202020
//   TextEdit.textForeground:      #ffffff
//   TextEdit.textSelection:       #6AA835
//   TextEdit.menuBackground:      #3c3b37
//   TextEdit.menuForeground:      #ffffff
//   TextEdit.menuSelection:       #6AA835
//   TextEdit.checkboxBackground:  #3c3b37
//   TextEdit.checkboxSelection:   #6AA835
//   TextEdit.browserSelection:    #6AA835
//   TextEdit.inputSelection:      #6AA835
//   TextEdit.scrollbarBackground: #3c3b37
//   TextEdit.scrollbarArrow:      #3c3b37
//   TextEdit.cjkFont:             WenQuanYi Micro Hei
//
// The parser also accepts XMenu.<key> aliases, for example:
//   XMenu.background:             #3c3b37
// --------------------------------------------------------------------- //

struct AppColors {
  Fl_Color background, background2, foreground, selection;
  Fl_Color text_background, text_foreground, text_selection;
  Fl_Color menu_background, menu_foreground, menu_selection;
  Fl_Color checkbox_background, checkbox_selection;
  Fl_Color browser_selection, input_selection;
  Fl_Color scrollbar_background, scrollbar_arrow;
  int have_background, have_background2, have_foreground, have_selection;
  int have_text_background, have_text_foreground, have_text_selection;
  int have_menu_background, have_menu_foreground, have_menu_selection;
  int have_checkbox_background, have_checkbox_selection;
  int have_browser_selection, have_input_selection;
  int have_scrollbar_background, have_scrollbar_arrow;
};

static AppColors app_colors = {0};
static char app_cjk_font_name[256] = "WenQuanYi Micro Hei";

static char *trim_space(char *s) {
  while (*s && isspace((unsigned char)*s)) ++s;
  char *end = s + strlen(s);
  while (end > s && isspace((unsigned char)end[-1])) --end;
  *end = 0;
  return s;
}

static int hex_value(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static int parse_hex_color(const char *s, Fl_Color *out) {
  if (!s || s[0] != '#') return 0;
  int len = strlen(s + 1);
  unsigned int r = 0, g = 0, b = 0;
  if (len == 6) {
    int v[6];
    for (int i = 0; i < 6; ++i) {
      v[i] = hex_value((unsigned char)s[i + 1]);
      if (v[i] < 0) return 0;
    }
    r = (v[0] << 4) | v[1];
    g = (v[2] << 4) | v[3];
    b = (v[4] << 4) | v[5];
  } else if (len == 3) {
    int v[3];
    for (int i = 0; i < 3; ++i) {
      v[i] = hex_value((unsigned char)s[i + 1]);
      if (v[i] < 0) return 0;
    }
    r = (v[0] << 4) | v[0];
    g = (v[1] << 4) | v[1];
    b = (v[2] << 4) | v[2];
  } else return 0;
  *out = fl_rgb_color((uchar)r, (uchar)g, (uchar)b);
  return 1;
}

static const char *resource_name_tail(const char *key) {
  const char *dot = strrchr(key, '.');
  return (dot && dot[1]) ? dot + 1 : key;
}

static void set_string_resource(const char *key, const char *value) {
  const char *name = resource_name_tail(key);
  if (!strcmp(name, "cjkFont") && value && *value) {
    strncpy(app_cjk_font_name, value, sizeof(app_cjk_font_name) - 1);
    app_cjk_font_name[sizeof(app_cjk_font_name) - 1] = 0;
  }
}

static void set_color_resource(const char *key, Fl_Color color) {
  const char *name = resource_name_tail(key);
  if (!strcmp(name, "dialogBackground")) { app_colors.background = color; app_colors.have_background = 1; }
  else if (!strcmp(name, "fieldBackground")) { app_colors.background2 = color; app_colors.have_background2 = 1; }
  else if (!strcmp(name, "foreground")) { app_colors.foreground = color; app_colors.have_foreground = 1; }
  else if (!strcmp(name, "selection")) { app_colors.selection = color; app_colors.have_selection = 1; }
  else if (!strcmp(name, "textBackground")) { app_colors.text_background = color; app_colors.have_text_background = 1; }
  else if (!strcmp(name, "textForeground")) { app_colors.text_foreground = color; app_colors.have_text_foreground = 1; }
  else if (!strcmp(name, "textSelection")) { app_colors.text_selection = color; app_colors.have_text_selection = 1; }
  else if (!strcmp(name, "menuBackground")) { app_colors.menu_background = color; app_colors.have_menu_background = 1; }
  else if (!strcmp(name, "menuForeground")) { app_colors.menu_foreground = color; app_colors.have_menu_foreground = 1; }
  else if (!strcmp(name, "menuSelection")) { app_colors.menu_selection = color; app_colors.have_menu_selection = 1; }
  else if (!strcmp(name, "checkboxBackground")) { app_colors.checkbox_background = color; app_colors.have_checkbox_background = 1; }
  else if (!strcmp(name, "checkboxSelection")) { app_colors.checkbox_selection = color; app_colors.have_checkbox_selection = 1; }
  else if (!strcmp(name, "browserSelection")) { app_colors.browser_selection = color; app_colors.have_browser_selection = 1; }
  else if (!strcmp(name, "inputSelection")) { app_colors.input_selection = color; app_colors.have_input_selection = 1; }
  else if (!strcmp(name, "scrollbarBackground")) { app_colors.scrollbar_background = color; app_colors.have_scrollbar_background = 1; }
  else if (!strcmp(name, "scrollbarArrow")) { app_colors.scrollbar_arrow = color; app_colors.have_scrollbar_arrow = 1; }
}

static void load_color_resources_from_file(const char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) return;
  char line[512];
  while (fgets(line, sizeof(line), fp)) {
    char *p = trim_space(line);
    if (!*p || *p == '!') continue;
    char *colon = strchr(p, ':');
    if (!colon) continue;
    *colon = 0;
    char *key = trim_space(p);
    char *value = trim_space(colon + 1);
    if (!strncmp(key, "TextEdit.", 9) || !strncmp(key, "XMenu.", 6)) {
      if (!strcmp(resource_name_tail(key), "cjkFont")) {
        set_string_resource(key, value);
        continue;
      }
      Fl_Color color;
      if (!parse_hex_color(value, &color)) continue;
      set_color_resource(key, color);
    }
  }
  fclose(fp);
}

static void init_app_colors() {
  load_color_resources_from_file("/usr/share/X11/app-defaults/TextEdit");
  if (app_colors.have_background) { uchar r, g, b; Fl::get_color(app_colors.background, r, g, b); Fl::background(r, g, b); }
  if (app_colors.have_background2) { uchar r, g, b; Fl::get_color(app_colors.background2, r, g, b); Fl::background2(r, g, b); }
  if (app_colors.have_foreground) { uchar r, g, b; Fl::get_color(app_colors.foreground, r, g, b); Fl::foreground(r, g, b); }
  if (app_colors.have_selection) Fl::set_color(FL_SELECTION_COLOR, app_colors.selection);
  if (app_colors.have_menu_selection) Fl::set_color(FL_SELECTION_COLOR, app_colors.menu_selection);
}

static int have_dialog_background_color() {
  return app_colors.have_background;
}

static Fl_Color dialog_background_color() {
  return app_colors.background;
}

static int have_field_background_color() {
  return app_colors.have_background2;
}

static Fl_Color field_background_color() {
  return app_colors.background2;
}

static void apply_app_colors(Fl_Widget *w) {
  if (!w) return;

  // TextEdit.dialogBackground is the general window/dialog background.
  // TextEdit.fieldBackground is used for text/list/input backgrounds.
  // Menus are controlled only by TextEdit.menu* resources.
  if (have_dialog_background_color()) w->color(dialog_background_color());
  if (app_colors.have_foreground) w->labelcolor(app_colors.foreground);
  if (app_colors.have_selection) w->selection_color(app_colors.selection);

  Fl_Text_Display *td = dynamic_cast<Fl_Text_Display *>(w);
  if (td) {
    if (app_colors.have_text_background) td->color(app_colors.text_background);
    else if (have_field_background_color()) td->color(field_background_color());
    if (app_colors.have_text_foreground) td->textcolor(app_colors.text_foreground);
    if (app_colors.have_text_selection) td->selection_color(app_colors.text_selection);
  }

  Fl_Input_ *input = dynamic_cast<Fl_Input_ *>(w);
  if (input) {
    if (have_field_background_color()) input->color(field_background_color());
    if (app_colors.have_input_selection) input->selection_color(app_colors.input_selection);
  }

  Fl_Browser_ *browser = dynamic_cast<Fl_Browser_ *>(w);
  if (browser) {
    if (have_field_background_color()) browser->color(field_background_color());
    if (app_colors.have_browser_selection) browser->selection_color(app_colors.browser_selection);
  }

  Fl_Scrollbar *scrollbar = dynamic_cast<Fl_Scrollbar *>(w);
  if (scrollbar) {
    if (app_colors.have_scrollbar_background) scrollbar->color(app_colors.scrollbar_background);
    if (app_colors.have_scrollbar_arrow) scrollbar->labelcolor(app_colors.scrollbar_arrow);
    else if (app_colors.have_scrollbar_background) scrollbar->labelcolor(app_colors.scrollbar_background);
  }

  Fl_Menu_ *menu = dynamic_cast<Fl_Menu_ *>(w);
  if (menu) {
    if (app_colors.have_menu_background) menu->color(app_colors.menu_background);
    if (app_colors.have_menu_foreground) menu->textcolor(app_colors.menu_foreground);
    if (app_colors.have_menu_selection) menu->selection_color(app_colors.menu_selection);
  }

  if (w->type() == FL_MENU_TOGGLE || w->type() == FL_MENU_RADIO) {
    if (app_colors.have_checkbox_background) w->color(app_colors.checkbox_background);
    if (app_colors.have_checkbox_selection) w->selection_color(app_colors.checkbox_selection);
  }

  Fl_Group *g = dynamic_cast<Fl_Group *>(w);
  if (g) for (int i = 0; i < g->children(); ++i) apply_app_colors(g->child(i));
}

static void init_app_fonts() {
  Fl::set_fonts("*");

  Fl::set_font(FL_HELVETICA, "DejaVu Sans:style=Book");
  Fl::set_font(FL_HELVETICA_BOLD, "DejaVu Sans:style=Bold");
  Fl::set_font(FL_HELVETICA_ITALIC, "DejaVu Sans:style=Oblique");
  Fl::set_font(FL_HELVETICA_BOLD_ITALIC, "DejaVu Sans:style=Bold Oblique");

  Fl::set_font(FL_COURIER, "DejaVu Sans Mono:style=Book");
  Fl::set_font(FL_COURIER_BOLD, "DejaVu Sans Mono:style=Bold");
  Fl::set_font(FL_COURIER_ITALIC, "DejaVu Sans Mono:style=Oblique");
  Fl::set_font(FL_COURIER_BOLD_ITALIC, "DejaVu Sans Mono:style=Bold Oblique");

  Fl::set_font(FL_SCREEN, "DejaVu Sans:style=Book");
  Fl::set_font(FL_SCREEN_BOLD, "DejaVu Sans:style=Bold");
  Fl::set_font(Editor_CJKFont, app_cjk_font_name);

  FL_NORMAL_SIZE = APP_FONT_SIZE;
}

static void init_common_dialog_labels() {
  fl_ok = tr("OK");
  fl_cancel = tr("Cancel");
  fl_yes = tr("Yes");
  fl_no = tr("No");
  fl_close = tr("Close");
  fl_file_chooser_ok_label(tr("OK"));

  Fl_File_Chooser::preview_label = tr("Preview");
  Fl_File_Chooser::new_directory_label = tr("New Directory?");
  Fl_File_Chooser::new_directory_tooltip = tr("Create a new directory.");
  Fl_File_Chooser::hidden_label = tr("Show hidden files");
  Fl_File_Chooser::show_label = tr("Show:");
  Fl_File_Chooser::filename_label = tr("Filename:");
  Fl_File_Chooser::all_files_label = tr("All Files (*)");
  Fl_File_Chooser::custom_filter_label = tr("Custom Filter");
  Fl_File_Chooser::favorites_label = tr("Favorites");
  Fl_File_Chooser::add_favorites_label = tr("Add to Favorites");
  Fl_File_Chooser::manage_favorites_label = tr("Manage Favorites");
  Fl_File_Chooser::filesystems_label = tr("File Systems");
  Fl_File_Chooser::existing_file_label = tr("Please choose an existing file!");
}

static Fl_Window *app_dialog_parent() {
  for (Fl_Window *w = Fl::first_window(); w; w = Fl::next_window(w)) {
    if (w->shown()) return w;
  }
  return 0;
}

static void center_window_on_parent(Fl_Window *win, Fl_Window *parent) {
  if (!win) return;
  if (!parent) parent = app_dialog_parent();

  int x, y;
  if (parent) {
    x = parent->x() + (parent->w() - win->w()) / 2;
    y = parent->y() + (parent->h() - win->h()) / 2;
  } else {
    x = (Fl::w() - win->w()) / 2;
    y = (Fl::h() - win->h()) / 2;
  }

  if (x < 0) x = 0;
  if (y < 0) y = 0;
  win->position(x, y);
}

static Fl_Window *find_visible_window_by_label(const char *label) {
  if (!label) return 0;
  for (Fl_Window *w = Fl::first_window(); w; w = Fl::next_window(w)) {
    const char *wl = w->label();
    if (w->shown() && wl && strcmp(wl, label) == 0) return w;
  }
  return 0;
}

static void center_visible_window_by_label(const char *label, Fl_Window *parent) {
  Fl_Window *w = find_visible_window_by_label(label);
  if (w) {
    center_window_on_parent(w, parent);
    w->redraw();
  }
}

static void fit_filechooser_window_by_label(const char *label, Fl_Window *parent) {
  Fl_Window *w = find_visible_window_by_label(label);
  if (!w) return;

  int max_w = Fl::w();
  int max_h = Fl::h();

  if (max_w <= 0 || max_w > APP_FILE_CHOOSER_MAX_W) max_w = APP_FILE_CHOOSER_MAX_W;
  if (max_h <= 0 || max_h > APP_FILE_CHOOSER_MAX_H) max_h = APP_FILE_CHOOSER_MAX_H;

  int new_w = w->w();
  int new_h = w->h();
  if (new_w > max_w) new_w = max_w;
  if (new_h > max_h) new_h = max_h;

  if (new_w != w->w() || new_h != w->h()) {
    w->size(new_w, new_h);
  }

  center_window_on_parent(w, parent);

  if (w->x() < 0 || w->y() < 0) {
    int x = w->x() < 0 ? 0 : w->x();
    int y = w->y() < 0 ? 0 : w->y();
    w->position(x, y);
  }

  w->redraw();
}

static void apply_filechooser_selection_colors(Fl_Widget *w) {
  if (!w) return;

  Fl_Input_ *input = dynamic_cast<Fl_Input_ *>(w);
  if (input) {
    if (have_field_background_color()) input->color(field_background_color());
    if (app_colors.have_input_selection) input->selection_color(app_colors.input_selection);
    else if (app_colors.have_selection) input->selection_color(app_colors.selection);
  }

  Fl_Browser_ *browser = dynamic_cast<Fl_Browser_ *>(w);
  if (browser) {
    if (have_field_background_color()) browser->color(field_background_color());
    if (app_colors.have_browser_selection) browser->selection_color(app_colors.browser_selection);
    else if (app_colors.have_selection) browser->selection_color(app_colors.selection);
  }

  Fl_Scrollbar *scrollbar = dynamic_cast<Fl_Scrollbar *>(w);
  if (scrollbar) {
    if (app_colors.have_scrollbar_background) scrollbar->color(app_colors.scrollbar_background);
    if (app_colors.have_scrollbar_arrow) scrollbar->labelcolor(app_colors.scrollbar_arrow);
    else if (app_colors.have_scrollbar_background) scrollbar->labelcolor(app_colors.scrollbar_background);
  }

  Fl_Group *g = dynamic_cast<Fl_Group *>(w);
  if (g) for (int i = 0; i < g->children(); ++i) apply_filechooser_selection_colors(g->child(i));
}

static void apply_filechooser_selection_colors_by_label(const char *label) {
  Fl_Window *w = find_visible_window_by_label(label);
  if (w) {
    apply_filechooser_selection_colors(w);
    w->redraw();
  }
}

static void centered_button_cb(Fl_Widget *w, void *data) {
  int *value = (int *)data;
  *value = (int)(long)w->user_data();
  w->window()->hide();
}

static int centered_choice(Fl_Window *parent, const char *message,
                           const char *b0, const char *b1, const char *b2) {
  int result = -1;
  Fl_Window dlg(380, 130, "");
  dlg.set_modal();
  dlg.labelfont(APP_FONT);
  dlg.labelsize(APP_FONT_SIZE);

  Fl_Box msg(15, 12, 350, 55, message);
  msg.labelfont(APP_FONT);
  msg.labelsize(APP_FONT_SIZE);
  msg.align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  Fl_Button cancel(15, 92, 110, 26, b0);
  cancel.labelfont(APP_FONT);
  cancel.labelsize(APP_FONT_SIZE);
  cancel.user_data((void *)0);
  cancel.callback(centered_button_cb, &result);

  Fl_Return_Button save(135, 92, 110, 26, b1);
  save.labelfont(APP_FONT);
  save.labelsize(APP_FONT_SIZE);
  save.user_data((void *)1);
  save.callback(centered_button_cb, &result);

  Fl_Button dontsave(255, 92, 110, 26, b2);
  dontsave.labelfont(APP_FONT);
  dontsave.labelsize(APP_FONT_SIZE);
  dontsave.user_data((void *)2);
  dontsave.callback(centered_button_cb, &result);

  apply_app_colors(&dlg);
  dlg.end();
  center_window_on_parent(&dlg, parent);
  dlg.show();
  while (dlg.shown()) Fl::wait();
  return result;
}

static void centered_input_ok_cb(Fl_Widget *w, void *data) {
  int *value = (int *)data;
  *value = 1;
  w->window()->hide();
}

static void centered_input_cancel_cb(Fl_Widget *w, void *data) {
  int *value = (int *)data;
  *value = 0;
  w->window()->hide();
}

static const char *centered_input(Fl_Window *parent, const char *message, const char *defval) {
  static char value[256];
  int result = 0;
  value[0] = '\0';
  if (defval) strncpy(value, defval, sizeof(value) - 1);
  value[sizeof(value) - 1] = '\0';

  Fl_Window dlg(360, 110, "");
  dlg.set_modal();
  dlg.labelfont(APP_FONT);
  dlg.labelsize(APP_FONT_SIZE);

  Fl_Input input(95, 15, 250, 25, message);
  input.labelfont(APP_FONT);
  input.labelsize(APP_FONT_SIZE);
  input.textfont(APP_FONT);
  input.textsize(APP_FONT_SIZE);
  input.value(value);

  Fl_Button cancel(160, 70, 85, 26, tr("Cancel"));
  cancel.labelfont(APP_FONT);
  cancel.labelsize(APP_FONT_SIZE);
  cancel.callback(centered_input_cancel_cb, &result);

  Fl_Return_Button ok(260, 70, 85, 26, tr("OK"));
  ok.labelfont(APP_FONT);
  ok.labelsize(APP_FONT_SIZE);
  ok.callback(centered_input_ok_cb, &result);

  apply_app_colors(&dlg);
  dlg.end();
  center_window_on_parent(&dlg, parent);
  dlg.show();
  input.take_focus();
  while (dlg.shown()) Fl::wait();

  if (!result) return 0;
  strncpy(value, input.value(), sizeof(value) - 1);
  value[sizeof(value) - 1] = '\0';
  return value;
}

static void centered_message(Fl_Window *parent, const char *message) {
  int result = 0;
  Fl_Window dlg(360, 120, "");
  dlg.set_modal();
  dlg.labelfont(APP_FONT);
  dlg.labelsize(APP_FONT_SIZE);

  Fl_Box msg(15, 15, 330, 55, message);
  msg.labelfont(APP_FONT);
  msg.labelsize(APP_FONT_SIZE);
  msg.align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  Fl_Return_Button ok(135, 82, 90, 26, tr("OK"));
  ok.labelfont(APP_FONT);
  ok.labelsize(APP_FONT_SIZE);
  ok.callback(centered_input_ok_cb, &result);

  apply_app_colors(&dlg);
  dlg.end();
  center_window_on_parent(&dlg, parent);
  dlg.show();
  while (dlg.shown()) Fl::wait();
}

static const char *app_file_chooser_start_path(const char *fname) {
  if (fname && fname[0]) return fname;

  const char *home = getenv("HOME");
  if (home && home[0]) return home;

  return "/root";
}

static char *app_file_chooser(const char *message, const char *pattern, const char *fname, int type) {
  Fl_Window *parent = app_dialog_parent();
  const char *start_path = app_file_chooser_start_path(fname);
  Fl_File_Chooser chooser(start_path, pattern, type, message);
  chooser.rescan();
  chooser.show();

  /* The file chooser window is created asynchronously by FLTK.  Wait only
     until it exists, then adjust it once.  Do not repeat resize/color/redraw
     in the event loop: on Xfbdev the mouse cursor is software-rendered, so
     redrawing the window under the cursor on every mouse event makes the
     cursor visibly flicker. */
  while (chooser.shown() && !find_visible_window_by_label(message)) {
    Fl::wait();
  }

  fit_filechooser_window_by_label(message, parent);
  apply_filechooser_selection_colors_by_label(message);

  Fl_Window *chooser_win = find_visible_window_by_label(message);
  Fl_Window *manage_favorites_win = 0;
  const char *manage_favorites_label = tr("Manage Favorites");
  int manage_favorites_last_w = -1;
  int manage_favorites_last_h = -1;

  while (chooser.shown()) {
    Fl::wait();

    /* The Manage Favorites dialog is created internally by Fl_File_Chooser,
       so it is not centered by our normal dialog helpers.  Its final geometry
       can be established after the window first appears; therefore center it
       when it appears and again only if FLTK changes its size.  Do not redraw
       or reposition it on every event, to avoid cursor flicker on Xfbdev. */
    Fl_Window *w = find_visible_window_by_label(manage_favorites_label);
    if (w) {
      if (!chooser_win || !chooser_win->shown()) chooser_win = find_visible_window_by_label(message);
      if (w != manage_favorites_win ||
          w->w() != manage_favorites_last_w ||
          w->h() != manage_favorites_last_h) {
        Fl::flush();
        center_window_on_parent(w, chooser_win ? chooser_win : parent);
        manage_favorites_win = w;
        manage_favorites_last_w = w->w();
        manage_favorites_last_h = w->h();
      }
    } else {
      manage_favorites_win = 0;
      manage_favorites_last_w = -1;
      manage_favorites_last_h = -1;
    }
  }

  const char *value = chooser.value();
  return value ? strdup(value) : 0;
}

int                changed = 0;
char               filename[256] = "";
char               title[256];
Fl_Text_Buffer     *textbuf = 0;
Fl_Text_Buffer     *stylebuf = 0;

static Fl_Text_Display::Style_Table_Entry text_style_table[] = {
  { FL_BLACK, Editor_TextFont, Editor_TextSize },
  { FL_BLACK, Editor_CJKFont, Editor_TextSize }
};

static unsigned int utf8_decode_char(const char *s, int len, int *used) {
  const unsigned char *p = (const unsigned char *)s;
  if (len <= 0) { *used = 0; return 0; }
  if (p[0] < 0x80) { *used = 1; return p[0]; }
  if ((p[0] & 0xe0) == 0xc0 && len >= 2 && (p[1] & 0xc0) == 0x80) {
    *used = 2;
    return ((unsigned int)(p[0] & 0x1f) << 6) | (unsigned int)(p[1] & 0x3f);
  }
  if ((p[0] & 0xf0) == 0xe0 && len >= 3 &&
      (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80) {
    *used = 3;
    return ((unsigned int)(p[0] & 0x0f) << 12) |
           ((unsigned int)(p[1] & 0x3f) << 6) |
           (unsigned int)(p[2] & 0x3f);
  }
  if ((p[0] & 0xf8) == 0xf0 && len >= 4 &&
      (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80 && (p[3] & 0xc0) == 0x80) {
    *used = 4;
    return ((unsigned int)(p[0] & 0x07) << 18) |
           ((unsigned int)(p[1] & 0x3f) << 12) |
           ((unsigned int)(p[2] & 0x3f) << 6) |
           (unsigned int)(p[3] & 0x3f);
  }
  *used = 1;
  return p[0];
}

static int is_cjk_codepoint(unsigned int cp) {
  return (cp >= 0x3400 && cp <= 0x4dbf) ||
         (cp >= 0x4e00 && cp <= 0x9fff) ||
         (cp >= 0xf900 && cp <= 0xfaff) ||
         (cp >= 0x3000 && cp <= 0x303f) ||
         (cp >= 0xff00 && cp <= 0xffef);
}

static char style_for_codepoint(unsigned int cp) {
  return is_cjk_codepoint(cp) ? 'B' : 'A';
}

static void restyle_text_buffer() {
  if (!textbuf || !stylebuf) return;
  int len = textbuf->length();
  char *text = textbuf->text();
  if (!text) return;
  char *styles = (char *)malloc((size_t)len + 1);
  if (!styles) { free(text); return; }

  int i = 0;
  while (i < len) {
    int used = 1;
    unsigned int cp = utf8_decode_char(text + i, len - i, &used);
    if (used <= 0 || i + used > len) used = 1;
    char st = style_for_codepoint(cp);
    for (int j = 0; j < used && i + j < len; ++j) styles[i + j] = st;
    i += used;
  }
  styles[len] = 0;
  stylebuf->text(styles);
  free(styles);
  free(text);
}


// Editor window functions and class...

void save_cb();
void saveas_cb();
void find2_cb(Fl_Widget*, void*);
void replall_cb(Fl_Widget*, void*);
void replace2_cb(Fl_Widget*, void*);
void replcan_cb(Fl_Widget*, void*);


class EditorWindow : public Fl_Double_Window {
  public:
    EditorWindow(int w, int h, const char* t);
    ~EditorWindow();

    Fl_Window          *replace_dlg;
    Fl_Input           *replace_find;
    Fl_Input           *replace_with;
    Fl_Button          *replace_all;
    Fl_Return_Button   *replace_next;
    Fl_Button          *replace_cancel;

#ifdef ENA_LINEWRAP
    int			wrap_mode;
#endif // ENA_LINEWRAP

#ifdef ENA_LINENUMS
    int			line_numbers;
#endif // ENA_LINENUMS

    Fl_Text_Editor     *editor;
    char               search[256];
};

class LocalizedTextEditor : public Fl_Text_Editor {
  public:
    LocalizedTextEditor(int x, int y, int w, int h)
      : Fl_Text_Editor(x, y, w, h) {}

    int handle(int event) {
      if (event == FL_PUSH && Fl::event_button() == FL_RIGHT_MOUSE) {
        Fl_Menu_Item context_menu[] = {
          { tr("Cu&t"),    0, 0, (void*)1 },
          { tr("&Copy"),   0, 0, (void*)2 },
          { tr("&Paste"),  0, 0, (void*)3 },
          { 0 }
        };

        const Fl_Menu_Item *picked = context_menu->popup(Fl::event_x(), Fl::event_y());
        if (!picked) return 1;

        switch ((long)picked->user_data()) {
          case 1:
            Fl_Text_Editor::kf_cut(0, this);
            break;
          case 2:
            Fl_Text_Editor::kf_copy(0, this);
            break;
          case 3:
            Fl_Text_Editor::kf_paste(0, this);
            break;
        }
        return 1;
      }

      return Fl_Text_Editor::handle(event);
    }
};

EditorWindow::EditorWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) {
  labelfont(APP_FONT);
  labelsize(APP_FONT_SIZE);
  replace_dlg = new Fl_Window(430, 115, tr("Replace"));
  replace_dlg->labelfont(APP_FONT);
  replace_dlg->labelsize(APP_FONT_SIZE);
    replace_find = new Fl_Input(105, 10, 315, 25, tr("Find:"));
    replace_find->labelfont(APP_FONT);
    replace_find->labelsize(APP_FONT_SIZE);
    replace_find->textfont(APP_FONT);
    replace_find->textsize(APP_FONT_SIZE);
    replace_find->align(FL_ALIGN_LEFT);

    replace_with = new Fl_Input(105, 40, 315, 25, tr("Replace:"));
    replace_with->labelfont(APP_FONT);
    replace_with->labelsize(APP_FONT_SIZE);
    replace_with->textfont(APP_FONT);
    replace_with->textsize(APP_FONT_SIZE);
    replace_with->align(FL_ALIGN_LEFT);

    replace_all = new Fl_Button(10, 78, 125, 27, tr("Replace All"));
    replace_all->labelfont(APP_FONT);
    replace_all->labelsize(APP_FONT_SIZE);
    replace_all->callback((Fl_Callback *)replall_cb, this);

    replace_next = new Fl_Return_Button(145, 78, 155, 27, tr("Replace Next"));
    replace_next->labelfont(APP_FONT);
    replace_next->labelsize(APP_FONT_SIZE);
    replace_next->callback((Fl_Callback *)replace2_cb, this);

    replace_cancel = new Fl_Button(310, 78, 110, 27, tr("Cancel"));
    replace_cancel->labelfont(APP_FONT);
    replace_cancel->labelsize(APP_FONT_SIZE);
    replace_cancel->callback((Fl_Callback *)replcan_cb, this);
  replace_dlg->end();
  replace_dlg->set_non_modal();
  apply_app_colors(replace_dlg);
  editor = 0;
  *search = (char)0;

#ifdef ENA_LINEWRAP
  wrap_mode = 0;
#endif // ENA_LINEWRAP

#ifdef ENA_LINENUMS
  line_numbers = 0;
#endif // ENA_LINENUMS
}

EditorWindow::~EditorWindow() {
  delete replace_dlg;
}

int check_save(void) {
  if (!changed) return 1;

  int r = centered_choice(app_dialog_parent(),
                          tr("The current file has not been saved.\nWould you like to save it now?"),
                          tr("Cancel"), tr("Save"), tr("Don't Save"));

  if (r == 1) {
    save_cb(); // Save the file...
    return !changed;
  }

  return (r == 2) ? 1 : 0;
}

int loading = 0;
void load_file(char *newfile, int ipos) {
  loading = 1;
  int insert = (ipos != -1);
  changed = insert;
  if (!insert) strcpy(filename, "");
  int r;
  if (!insert) r = textbuf->loadfile(newfile);
  else r = textbuf->insertfile(newfile, ipos);
  if (r) {
    char msg[512];
    snprintf(msg, sizeof(msg), tr("Error reading from file '%s':\n%s."), newfile, strerror(errno));
    centered_message(app_dialog_parent(), msg);
  }
  else
    if (!insert) strcpy(filename, newfile);
  loading = 0;
  textbuf->call_modify_callbacks();
}

void save_file(char *newfile) {
  if (textbuf->savefile(newfile)) {
    char msg[512];
    snprintf(msg, sizeof(msg), tr("Error writing to file '%s':\n%s."), newfile, strerror(errno));
    centered_message(app_dialog_parent(), msg);
  }
  else
    strcpy(filename, newfile);
  changed = 0;
  textbuf->call_modify_callbacks();
}

void copy_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Text_Editor::kf_copy(0, e->editor);
}

void cut_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Text_Editor::kf_cut(0, e->editor);
}

void delete_cb(Fl_Widget*, void*) {
  textbuf->remove_selection();
}

#ifdef ENA_LINEWRAP
void wordwrap_cb(Fl_Widget *w, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Menu_Bar* m = (Fl_Menu_Bar*)w;
  const Fl_Menu_Item* i = m->mvalue();
  if ( i->value() )
    e->editor->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
  else
    e->editor->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);
  e->wrap_mode = (i->value()?1:0);
  e->redraw();
}
#endif // ENA_LINEWRAP

#ifdef ENA_LINENUMS
void linenumbers_cb(Fl_Widget *w, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Menu_Bar* m = (Fl_Menu_Bar*)w;
  const Fl_Menu_Item* i = m->mvalue();
  if ( i->value() ) {
    e->editor->linenumber_width(line_num_width);	// enable
    e->editor->linenumber_font(e->editor->textfont());
    e->editor->linenumber_size(e->editor->textsize());
  } else {
    e->editor->linenumber_width(0);	// disable
  }
  e->line_numbers = (i->value()?1:0);
  e->redraw();
}
#endif // ENA_LINENUMS

void find_cb(Fl_Widget* w, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  const char *val;

  val = centered_input(e, tr("Search String:"), e->search);
  if (val != NULL) {
    // User entered a string - go find it!
    strcpy(e->search, val);
    find2_cb(w, v);
  }
}

void find2_cb(Fl_Widget* w, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  if (e->search[0] == '\0') {
    // Search string is blank; get a new one...
    find_cb(w, v);
    return;
  }

  int pos = e->editor->insert_position();
  int found = textbuf->search_forward(pos, e->search, &pos);
  if (found) {
    // Found a match; select and update the position...
    textbuf->select(pos, pos+strlen(e->search));
    e->editor->insert_position(pos+strlen(e->search));
    e->editor->show_insert_position();
  }
  else {
    char msg[512];
    snprintf(msg, sizeof(msg), tr("No occurrences of '%s' found!"), e->search);
    centered_message(e, msg);
  }
}

void set_title(Fl_Window* w) {
  if (filename[0] == '\0') strcpy(title, tr("Untitled"));
  else {
    char *slash;
    slash = strrchr(filename, '/');
#ifdef WIN32
    if (slash == NULL) slash = strrchr(filename, '\\');
#endif
    if (slash != NULL) strcpy(title, slash + 1);
    else strcpy(title, filename);
  }

  if (changed) strcat(title, tr(" (modified)"));

  w->label(title);
}

void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) {
  if (nInserted || nDeleted) restyle_text_buffer();
  if ((nInserted || nDeleted) && !loading) changed = 1;
  EditorWindow *w = (EditorWindow *)v;
  set_title(w);
  if (loading) w->editor->show_insert_position();
}

void new_cb(Fl_Widget*, void*) {
  if (!check_save()) return;

  filename[0] = '\0';
  textbuf->select(0, textbuf->length());
  textbuf->remove_selection();
  changed = 0;
  textbuf->call_modify_callbacks();
}

void open_cb(Fl_Widget*, void*) {
  if (!check_save()) return;

  char *newfile = app_file_chooser(tr("Open File?"), "*", filename, Fl_File_Chooser::SINGLE);
  if (newfile != NULL) { load_file(newfile, -1); free(newfile); }
}

void insert_cb(Fl_Widget*, void *v) {
  char *newfile = app_file_chooser(tr("Insert File?"), "*", filename, Fl_File_Chooser::SINGLE);
  EditorWindow *w = (EditorWindow *)v;
  if (newfile != NULL) { load_file(newfile, w->editor->insert_position()); free(newfile); }
}

void paste_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Text_Editor::kf_paste(0, e->editor);
}

int num_windows = 0;

void close_cb(Fl_Widget*, void* v) {
  Fl_Window* w = (Fl_Window*)v;
  if (num_windows == 1 && !check_save()) {
    return;
  }

  w->hide();
  textbuf->remove_modify_callback(changed_cb, w);
  Fl::delete_widget(w);
  num_windows--;
  if (!num_windows) exit(0);
}

void quit_cb(Fl_Widget*, void*) {
  if (changed && !check_save())
    return;

  exit(0);
}

void replace_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  center_window_on_parent(e->replace_dlg, e);
  e->replace_dlg->show();
}

void replace2_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  const char *find = e->replace_find->value();
  const char *replace = e->replace_with->value();

  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    center_window_on_parent(e->replace_dlg, e);
    e->replace_dlg->show();
    return;
  }

  e->replace_dlg->hide();

  int pos = e->editor->insert_position();
  int found = textbuf->search_forward(pos, find, &pos);

  if (found) {
    // Found a match; update the position and replace text...
    textbuf->select(pos, pos+strlen(find));
    textbuf->remove_selection();
    textbuf->insert(pos, replace);
    textbuf->select(pos, pos+strlen(replace));
    e->editor->insert_position(pos+strlen(replace));
    e->editor->show_insert_position();
  }
  else {
    char msg[512];
    snprintf(msg, sizeof(msg), tr("No occurrences of '%s' found!"), find);
    centered_message(e, msg);
  }
}

void replall_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  const char *find = e->replace_find->value();
  const char *replace = e->replace_with->value();

  find = e->replace_find->value();
  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    center_window_on_parent(e->replace_dlg, e);
    e->replace_dlg->show();
    return;
  }

  e->replace_dlg->hide();

  e->editor->insert_position(0);
  int times = 0;

  // Loop through the whole string
  for (int found = 1; found;) {
    int pos = e->editor->insert_position();
    found = textbuf->search_forward(pos, find, &pos);

    if (found) {
      // Found a match; update the position and replace text...
      textbuf->select(pos, pos+strlen(find));
      textbuf->remove_selection();
      textbuf->insert(pos, replace);
      e->editor->insert_position(pos+strlen(replace));
      e->editor->show_insert_position();
      times++;
    }
  }

  if (times) {
    char msg[256];
    snprintf(msg, sizeof(msg), tr("Replaced %d occurrences."), times);
    centered_message(e, msg);
  }
  else {
    char msg[512];
    snprintf(msg, sizeof(msg), tr("No occurrences of '%s' found!"), find);
    centered_message(e, msg);
  }
}

void replcan_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  e->replace_dlg->hide();
}

void save_cb() {
  if (filename[0] == '\0') {
    // No filename - get one!
    saveas_cb();
    return;
  }
  else save_file(filename);
}

void saveas_cb() {
  char *newfile;

  newfile = app_file_chooser(tr("Save File As?"), "*", filename, Fl_File_Chooser::CREATE);
  if (newfile != NULL) { save_file(newfile); free(newfile); }
}

Fl_Window* new_view();

void view_cb(Fl_Widget*, void*) {
  Fl_Window* w = new_view();
  w->show();
}

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static Fl_Menu_Item menuitems[] = {
    { 0,                        0, 0, 0, FL_SUBMENU },
    { 0,                      0, (Fl_Callback *)new_cb },
    { 0,                      FL_CTRL + 'o', (Fl_Callback *)open_cb },
    { 0,                      FL_CTRL + 'i', (Fl_Callback *)insert_cb, 0, FL_MENU_DIVIDER },
    { 0,                      FL_CTRL + 's', (Fl_Callback *)save_cb },
    { 0,                      FL_CTRL + FL_SHIFT + 's', (Fl_Callback *)saveas_cb, 0, FL_MENU_DIVIDER },
    { 0,                      FL_ALT + 'v', (Fl_Callback *)view_cb, 0 },
    { 0,                      FL_CTRL + 'w', (Fl_Callback *)close_cb, 0, FL_MENU_DIVIDER },
    { 0,                      FL_CTRL + 'q', (Fl_Callback *)quit_cb, 0 },
    { 0 },

  { 0, 0, 0, 0, FL_SUBMENU },
    { 0,                    FL_CTRL + 'x', (Fl_Callback *)cut_cb },
    { 0,                    FL_CTRL + 'c', (Fl_Callback *)copy_cb },
    { 0,                    FL_CTRL + 'v', (Fl_Callback *)paste_cb },
    { 0,                    0, (Fl_Callback *)delete_cb },

#ifdef ENA_LINENUMS
    { 0,                    FL_COMMAND + 'n', (Fl_Callback *)linenumbers_cb, 0, FL_MENU_TOGGLE },
#endif // ENA_LINENUMS

#ifdef ENA_LINEWRAP
    { 0,                    FL_COMMAND + 'l', (Fl_Callback *)wordwrap_cb, 0, FL_MENU_TOGGLE },
#endif // ENA_LINEWRAP

    { 0 },

  { 0, 0, 0, 0, FL_SUBMENU },
    { 0,                    FL_CTRL + 'f', (Fl_Callback *)find_cb },
    { 0,                    FL_CTRL + 'g', find2_cb },
    { 0,                    FL_CTRL + 'r', replace_cb },
    { 0,                    FL_CTRL + 't', replace2_cb },
    { 0 },

  { 0, 0, 0, 0, FL_SUBMENU },
    { Version },
    { 0 },

  { 0 }
};

static void init_menu_labels() {
  menuitems[0].text = tr("&File");
  menuitems[1].text = tr("&New File");
  menuitems[2].text = tr("&Open File...");
  menuitems[3].text = tr("&Insert File...");
  menuitems[4].text = tr("&Save File");
  menuitems[5].text = tr("Save File &As...");
  menuitems[6].text = tr("New &View");
  menuitems[7].text = tr("&Close View");
  menuitems[8].text = tr("E&xit");

  menuitems[10].text = tr("&Edit");
  menuitems[11].text = tr("Cu&t");
  menuitems[12].text = tr("&Copy");
  menuitems[13].text = tr("&Paste");
  menuitems[14].text = tr("&Delete");

#ifdef ENA_LINENUMS
  menuitems[15].text = tr("Line &Numbers ");
#endif // ENA_LINENUMS

#ifdef ENA_LINEWRAP
#ifdef ENA_LINENUMS
  menuitems[16].text = tr("Word Wrap");
#else
  menuitems[15].text = tr("Word Wrap");
#endif
#endif // ENA_LINEWRAP

#ifdef ENA_LINENUMS
#ifdef ENA_LINEWRAP
  menuitems[18].text = tr("&Search");
  menuitems[19].text = tr("&Find...");
  menuitems[20].text = tr("F&ind Again");
  menuitems[21].text = tr("&Replace...");
  menuitems[22].text = tr("Re&place Again");
  menuitems[24].text = tr("&Version");
#else
  menuitems[17].text = tr("&Search");
  menuitems[18].text = tr("&Find...");
  menuitems[19].text = tr("F&ind Again");
  menuitems[20].text = tr("&Replace...");
  menuitems[21].text = tr("Re&place Again");
  menuitems[23].text = tr("&Version");
#endif
#else
#ifdef ENA_LINEWRAP
  menuitems[17].text = tr("&Search");
  menuitems[18].text = tr("&Find...");
  menuitems[19].text = tr("F&ind Again");
  menuitems[20].text = tr("&Replace...");
  menuitems[21].text = tr("Re&place Again");
  menuitems[23].text = tr("&Version");
#else
  menuitems[16].text = tr("&Search");
  menuitems[17].text = tr("&Find...");
  menuitems[18].text = tr("F&ind Again");
  menuitems[19].text = tr("&Replace...");
  menuitems[20].text = tr("Re&place Again");
  menuitems[22].text = tr("&Version");
#endif
#endif
}
#pragma GCC diagnostic warning "-Wmissing-field-initializers"

Fl_Window* new_view() {
  EditorWindow* w = new EditorWindow(660, 400, title);
    w->begin();
    Fl_Menu_Bar* m = new Fl_Menu_Bar(0, 0, 660, 30);
    m->labelfont(APP_FONT);
    m->labelsize(APP_FONT_SIZE);
    m->textfont(APP_FONT);
    m->textsize(APP_FONT_SIZE);
    m->copy(menuitems, w);
    w->editor = new LocalizedTextEditor(0, 30, 660, 370);
    w->editor->buffer(textbuf);
    text_style_table[0].color = app_colors.have_text_foreground ? app_colors.text_foreground : FL_BLACK;
    text_style_table[1].color = app_colors.have_text_foreground ? app_colors.text_foreground : FL_BLACK;
    w->editor->highlight_data(stylebuf, text_style_table, 2, 'A', 0, 0);
    w->editor->textfont(Editor_TextFont);
    w->editor->textsize(Editor_TextSize);
    apply_app_colors(w);
  w->labelfont(APP_FONT);
  w->labelsize(APP_FONT_SIZE);
  w->end();
  w->resizable(w->editor);
  w->callback((Fl_Callback *)close_cb, w);

  textbuf->add_modify_callback(changed_cb, w);
  textbuf->call_modify_callbacks();
  num_windows++;
  return w;
}

int main(int argc, char **argv) {

  app_lang = detect_app_language();
  load_color_resources_from_file("/usr/share/X11/app-defaults/TextEdit");
  init_app_fonts();
  init_common_dialog_labels();
  init_menu_labels();
  Fl::scheme("gtk+");
  init_app_colors();
  Fl_Tooltip::font(APP_FONT);
  Fl_Tooltip::size(APP_FONT_SIZE);
  fl_message_font(APP_FONT, APP_FONT_SIZE);

#ifdef DIS_DNDTEXT
  Fl::dnd_text_ops(false);
#endif // DIS_DNDTEXT

  textbuf = new Fl_Text_Buffer;
  stylebuf = new Fl_Text_Buffer;
  restyle_text_buffer();

  Fl_Window* window = new_view();

  window->show(1, argv);

  if (argc > 1) load_file(argv[1], -1);

  return Fl::run();
}

//
// End of "$Id: editor.cxx 5519 2006-10-11 03:12:15Z mike $".
//
