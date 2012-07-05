#!/usr/bin/env python2.7

from operator import attrgetter, methodcaller

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.cm as cm

import wx
import wx.lib.newevent
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as Canvas


UpdateEvent, EVT_UPDATE_EVENT = wx.lib.newevent.NewCommandEvent()


def PackH(*widgets):
  sizer = wx.BoxSizer(wx.HORIZONTAL)
  for widget in widgets:
    sizer.Add(widget, 0, wx.ALIGN_CENTER_VERTICAL)
  return sizer


def PackVBox(parent, label, widgets):
  sizer = wx.StaticBoxSizer(wx.StaticBox(parent, label=label), wx.VERTICAL)
  for widget in widgets:
    sizer.Add(widget, 0, wx.EXPAND)
  return sizer


class SinModulator(wx.Panel):
  def __init__(self, parent, name, cycles, phase):
    wx.Panel.__init__(self, parent)

    self._active = wx.CheckBox(self, label="Active?")
    self._cycles = wx.SpinCtrl(self, min=cycles[0], max=cycles[1])
    self._phase = wx.Slider(self, minValue=phase[0], maxValue=phase[1])
    self._phaseLabel = wx.StaticText(self, label=self.phaseAsString, style=wx.ALIGN_CENTRE)

    self.Bind(wx.EVT_CHECKBOX, self.OnActiveChange, self._active)
    self.Bind(wx.EVT_SPINCTRL, self.OnChange, self._cycles)
    self.Bind(wx.EVT_SCROLL, self.OnPhaseChange, self._phase)

    self._cycles.SetValue(cycles[0])
    self._phase.SetValue(phase[0])
    
    cycles = PackH(wx.StaticText(self, label='Cycles:'), self._cycles)
    phase = PackH(self._phase, self._phaseLabel)

    self.SetSizer(PackVBox(self, name, [self._active, cycles, phase]))

    self.Enable(False)

  cycles = property(lambda s: s._cycles.GetValue())
  active = property(lambda s: s._active.IsChecked())
  phase = property(lambda s: s._phase.GetValue() / 1000.0)
  phaseAsString = property(lambda s: '%.3f' % s.phase)

  def Enable(self, enable=True):
    map(methodcaller('Enable', enable), [self._cycles, self._phase])

  def OnActiveChange(self, evt):
    self.Enable(evt.Checked())
    self.OnChange(evt)

  def OnPhaseChange(self, evt):
    self._phaseLabel.SetLabel(self.phaseAsString)
    self.OnChange(evt)

  def OnChange(self, evt):
    wx.PostEvent(self.GetParent(), UpdateEvent(self.GetId()))


class Plot(wx.Panel):
  def __init__(self, parent, dpi=None, **kwargs):
    wx.Panel.__init__(self, parent, **kwargs)
    self.figure = mpl.figure.Figure(dpi=dpi, figsize=(2, 2))
    self.canvas = Canvas(self, wx.ID_ANY, self.figure)

    self.check = wx.CheckBox(self, wx.ID_ANY, "Show contour")
    self.Bind(wx.EVT_CHECKBOX, self.OnChange, self.check)

    self.distMod = SinModulator(self, 'Amplitude modulator', cycles=(2,100), phase=(0, 999))
    self.Bind(EVT_UPDATE_EVENT, self.OnChange, self.distMod)

    sizerV = wx.BoxSizer(wx.VERTICAL)
    sizerV.Add(self.distMod)
    sizerV.Add(self.check, 0, wx.EXPAND)

    sizerH = wx.BoxSizer(wx.HORIZONTAL)
    sizerH.Add(self.canvas, 0, wx.SHAPED | wx.EXPAND)
    sizerH.Add(sizerV)

    self.SetSizer(sizerH)
    
    self.width, self.height = 256, 256

  def CalcPixel(self, i, j):
    x = 2.0 * i / self.width - 1.0
    y = 2.0 * j / self.height - 1.0
    dist = np.sqrt(x**2 + y**2) 
    angle = np.arctan2(y, x)

    data = self.distMod

    if data.active:
      phase = data.phase * 2.0 * np.pi 
      newAngle = angle * data.cycles + phase
      distDiff = np.cos(newAngle) * dist / data.cycles
    else:
      distDiff = 0.0

    return 1.0 - (dist + distDiff) 

  def Draw(self):
    self.figure.clear()
    subplot = self.figure.add_subplot(111)

    x = np.arange(0.0, self.width, 1.0)
    y = np.arange(0.0, self.height, 1.0)
    I, J = np.meshgrid(x, y)
    C = np.clip(self.CalcPixel(I, J), 0.0, 1.0)

    if self.check.IsChecked():
      self.CS = subplot.contour(I, J, C)
      subplot.clabel(self.CS, inline=0.1, fontsize=8)

    im = subplot.imshow(C, cmap=cm.gray)
    im.set_interpolation('bilinear')

  def OnChange(self, _):
    self.Draw()
    self.canvas.draw()


class MainWindow(wx.Frame):
  def __init__(self, title):
    wx.Frame.__init__(self, None, title=title, size=(768, 512))

    self.CreateStatusBar()

    filemenu = wx.Menu()
    menuExit = filemenu.Append(wx.ID_EXIT, "E&xit"," Terminate the program")
    self.Bind(wx.EVT_MENU, self.OnExit, menuExit)

    menuBar = wx.MenuBar()
    menuBar.Append(filemenu, "&File")
    self.SetMenuBar(menuBar)

    self.plot = Plot(self)
    self.plot.Draw()
    self.Show()

  def OnExit(self, evt):
    self.Close()


if __name__ == '__main__':
  app = wx.PySimpleApp()
  MainWindow('Light texture')
  app.MainLoop()
