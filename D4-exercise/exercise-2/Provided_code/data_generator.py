#!/usr/bin/env python2

"""
This small program given one example netcdf file, generates another
file with some random noise for another date as in the input
"""

def generate_perturbe(datafile,year,month):
  """
Output file will be created in the current directory
  """
  from netCDF4 import Dataset
  import numpy as np
  import time
  from netcdftime import datetime , num2date , utime
  import calendar
  import os
  from string import join

  ncf = Dataset(datafile)

  times = ncf.variables['time']
  f1 = (repr(year).zfill(4)+repr(month).zfill(2)+repr(1).zfill(2))
  year1 = year
  month1 = month + 1
  if ( month1 == 13 ):
    year1 = year + 1
    month1 = month
  f2 = (repr(year1).zfill(4)+repr(month1).zfill(2)+repr(1).zfill(2))

  pieces = os.path.basename(os.path.splitext(datafile)[0]).split('_')
 
  nco = Dataset(join(pieces[0:8],'_')+'_'+f1+'0100-'+f2+'0100.nc',
                'w', format='NETCDF4_CLASSIC')
  tunit = 'hours since 1949-12-01 00:00:00 UTC'
  cdftime = utime(tunit,calendar=times.calendar)        

  for attr in ncf.ncattrs():
    nco.setncattr(attr,getattr(ncf,attr))

  for dim in ncf.dimensions:
    if ( ncf.dimensions[dim].isunlimited() ):
      nco.createDimension(dim)
    else:
      nco.createDimension(dim,len(ncf.dimensions[dim]))

  for var in ncf.variables:
    nctype = ncf.variables[var].datatype
    if ('x' in ncf.variables[var].dimensions and
        'y' in ncf.variables[var].dimensions):
      nco.createVariable(var,nctype,ncf.variables[var].dimensions,
                         shuffle=True,fletcher32=True,
                         zlib=True,complevel=9)
    else:
      nco.createVariable(var,nctype,ncf.variables[var].dimensions)
      for attr in ncf.variables[var].ncattrs():
        nco.variables[var].setncattr(attr,getattr(ncf.variables[var],attr))

  for var in ncf.variables:
    if 'time' not in ncf.variables[var].dimensions:
      nco.variables[var][:] = ncf.variables[var][:]
    else:
      it = 0
      if ( times.calendar == '360_day' ):
        dayr = (1,30)
      else:
        dayr = calendar.monthrange(year,month)
      for day in xrange(1,dayr[1]+1):
        for h in xrange(0,23,3):
          date = datetime(year,month,day,h)
          if var == 'time':
            rc = cdftime.date2num(date)
            nco.variables[var][it] = rc
          elif var == 'time_bnds':
            rc = cdftime.date2num(date)
            tb = np.array([0.0,0.0])
            tb[0] = rc - 3
            tb[1] = rc
            nco.variables[var][it,Ellipsis] = tb
          else:
            nums = ncf.variables[var][it,Ellipsis]
            xshape = np.shape(nums)
            dim = np.product(xshape)
            mxval = np.max(nums)
            mnval = np.min(nums)
            # Add a 2% noise
            nco.variables[var][it,Ellipsis] = (
                    nums + 2.0*np.random.sample(dim).reshape(xshape) *
                    ((mxval-mnval)/100.0))
          it = it + 1

  ncf.close()
  nco.close()

if ( __name__ == '__main__' ):
  import sys
  if len(sys.argv) < 4:
    print('Need file name')
    sys.exit(-1)
  infile = sys.argv[1]
  year = sys.argv[2]
  month = sys.argv[3]
  generate_perturbe(infile, int(year), int(month))
