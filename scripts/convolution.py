#!/usr/bin/arch -arch i386 python2.7

import pygame
import numpy as np
import numpy.fft as fft

pygame.surfarray.use_arraytype("numpy")

def MakeKernel(kind):
  if kind == 'gauss5x5':
    A = np.matrix([1, 4, 6, 4, 1])
    B = np.transpose(A)
    return B * A

  if kind == 'gauss3x3':
    A = np.matrix([1, 2, 1])
    B = np.transpose(A)
    return B * A

  if kind == 'edgeV':
    return np.matrix([[-1, -2, -1],
                      [ 0,  0,  0],
                      [ 1,  2,  1]])

  if kind == 'laplace':
    return np.matrix([[-1, -1, -1],
                      [-1,  8, -1],
                      [-1, -1, -1]])

def ExpandKernel(K_raw, dim):
  K_sum = sum(abs(i) for i in K_raw.flat)
  K = np.asarray(K_raw / float(K_sum))
  K_half = K.shape[0] / 2
  K_exp = np.zeros(dim)

  for y, row in enumerate(K):
    for x, num in enumerate(row):
      K_exp[x - K_half, y - K_half] = num

  return K_exp

def MakeKernelFFT(kind, dim):
  K = MakeKernel(kind)
  return fft.fft2(ExpandKernel(K, dim))

def ApplyConvolution(kernel, image):
  P_i = np.transpose(image, [2, 0, 1])
  X = fft.fft2(P_i)
  Y = fft.ifft2(X * kernel)
  P_o = np.asfarray(np.real(Y))
  return np.uint8(np.transpose(P_o, [1, 2, 0]))

if __name__ == '__main__':
  pygame.init()

  image = pygame.image.load("artwork/textures/08.png")
  size = image.get_size()

  if all(d <= 256 for d in size):
    size = [d * 2 for d in size]

  screen = pygame.display.set_mode(size)
  array = pygame.surfarray.array3d(image)
  frame = image.copy()
  kernel = MakeKernelFFT('gauss3x3', image.get_size())

  while True:
    if any(ev.type == pygame.QUIT for ev in pygame.event.get()):
      break

    array = ApplyConvolution(kernel, array)

    pygame.surfarray.blit_array(frame, array)
    screen.blit(pygame.transform.scale(frame, size), (0, 0))
    pygame.display.flip()
