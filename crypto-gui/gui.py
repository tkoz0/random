import wx
from crypto import CryptoFunctions
from crypto import InvalidTextException

class CryptoFrame(wx.Frame):
    '''
    self.plaintext = plaintext text box
    self.ciphertext = ciphertext text box
    '''
    
    def __init__(self):
        super().__init__(parent=None,title='crypto gui for rot13')
        self.make_panel()
        self.make_menu()
        self.Show()
    
    def make_panel(self):
        panel = wx.Panel(self)
        sizer = wx.BoxSizer(wx.VERTICAL)
        
        # buttons
        buttonsizer = wx.BoxSizer(wx.HORIZONTAL)
        encbutton = wx.Button(panel,label='encrypt')
        decbutton = wx.Button(panel,label='decrypt')
        self.Bind(wx.EVT_BUTTON,self.encrypt,encbutton)
        self.Bind(wx.EVT_BUTTON,self.decrypt,decbutton)
        buttonsizer.Add(encbutton,0,wx.RIGHT|wx.CENTER,5)
        buttonsizer.Add(decbutton,0,wx.LEFT|wx.CENTER,5)
        
        # text boxes
        plainsizer = wx.BoxSizer(wx.HORIZONTAL)
        ciphersizer = wx.BoxSizer(wx.HORIZONTAL)
        self.plaintext = wx.TextCtrl(panel,wx.ID_ANY)
        self.ciphertext = wx.TextCtrl(panel,wx.ID_ANY)
        plainstatic = wx.StaticText(panel,wx.ID_ANY,'plaintext')
        cipherstatic = wx.StaticText(panel,wx.ID_ANY,'ciphertext')
        # the 0 (proportion) means static text size cant change
        plainsizer.Add(plainstatic,0,wx.TOP|wx.RIGHT,5)
        ciphersizer.Add(cipherstatic,0,wx.TOP|wx.RIGHT,5)
        # the 1 (proportion) means the text box can change size to fill space
        plainsizer.Add(self.plaintext,1,wx.CENTER|wx.LEFT|wx.EXPAND,5)
        ciphersizer.Add(self.ciphertext,1,wx.CENTER|wx.LEFT|wx.EXPAND,5)
        
        # finalize
        sizer.Add(buttonsizer,0,wx.ALL|wx.CENTER,0)
        sizer.Add(plainsizer,0,wx.LEFT|wx.RIGHT|wx.TOP|wx.EXPAND,5)
        sizer.Add(ciphersizer,0,wx.LEFT|wx.RIGHT|wx.TOP|wx.EXPAND,5)
        panel.SetSizer(sizer)
    
    def make_menu(self):
        menu = wx.MenuBar()
        file_menu = wx.Menu()
        help_menu = wx.Menu()
        exit_item = file_menu.Append(wx.ID_ANY,'exit','closes program')
        about_item = help_menu.Append(wx.ID_ANY,'about','program info')
        self.Bind(wx.EVT_MENU,handler=self.exit,source=exit_item)
        self.Bind(wx.EVT_MENU,handler=self.about,source=about_item)
        menu.Append(file_menu,'&file')
        menu.Append(help_menu,'&help')
        self.SetMenuBar(menu)
    
    def exit(self,event):
        self.Destroy()
    
    def about(self,event):
        msg = wx.MessageDialog(self,'basic gui for rot13 cipher')
        msg.ShowModal()
        msg.Destroy()
    
    def encrypt(self,event):
        try:
            plain = self.plaintext.GetValue()
            cipher = CryptoFunctions.encrypt_rot13(plain)
            self.ciphertext.SetValue(cipher)
        except:
            msg = wx.MessageDialog(self,'allowed chars: a-z, A-Z, and spaces')
            msg.ShowModal()
            msg.Destroy()
    
    def decrypt(self,event):
        try:
            cipher = self.ciphertext.GetValue()
            plain = CryptoFunctions.decrypt_rot13(cipher)
            self.plaintext.SetValue(plain)
        except:
            msg = wx.MessageDialog(self,'allowed chars: a-z, A-Z, and spaces')
            msg.ShowModal()
            msg.Destroy()

if __name__ == '__main__':
    app = wx.App()
    frame = CryptoFrame()
    app.MainLoop()
