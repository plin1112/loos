/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo, Alan Grossfield
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

%module Trajectory;
%include "std_vector.i"
%include "std_string.i"
%include "boost_shared_ptr.i"

%shared_ptr(loos::Trajectory)

%include "AtomicGroup.i"

%{
#include <Trajectory.hpp>
#include <StreamWrapper.hpp>
%}

namespace loos {

  class Trajectory;
  typedef boost::shared_ptr<Trajectory> pTraj;

  //! Base-class for polymorphic trajectories.
  /** This is the interface for trajectories in LOOS.  It is expected
   *  that at least one frame of coordinates will be buffered internally
   *  at any given time.
   *
   *  Additionally, this class is not designed to provide output, only
   *  input...
   *
   *  +IMPORTANT NOTE+
   *  The derived classes MUST read in and cache the first frame as
   *  part of their initialization.  This prevents problems where
   *  updateGroupCoords() is called prior to the class reading any
   *  trajectory data (which can occur with some formats, such as
   *  DCD's, that only have to read a header to configure the internal
   *  data...  However, just inserting a readFrame(0) in the
   *  constructor will leave the trajectory iterator in an incorrect
   *  state--the first call to readFrame() will return the 2nd frame,
   *  not the first, which is probably not the desired behavior.  The
   *  derived class must also then set the cached_first flag to true
   *  after the readFrame(0).  See the DCD class for an example of
   *  this.
   */

  class Trajectory : public boost::noncopyable {
  public:
    Trajectory() : cached_first(false) { }

    //! Automatically open the file named \a s
    Trajectory(const std::string& s) : ifs(s), cached_first(false) { }

    //! Automatically open the file named \a s
    Trajectory(const char* s) : ifs(s), cached_first(false) { }

    //! Open using the given stream...
    Trajectory(std::iostream& fs) : ifs(fs), cached_first(false) { }

    virtual ~Trajectory() { }

    //! # of atoms per frame
    virtual uint natoms(void) const =0;
    //! Timestep per frame
    virtual float timestep(void) const =0;
    //! Number of frames in the trajectory
    virtual uint nframes(void) const =0;

    //! Rewinds the readFrame() iterator
    void rewind(void);

    //! Tests whether or not the given frame/trajectory has periodic
    //! boundary information.
    /** The presence of periodic box information does not necessarily
     * indicate that said information has been read in yet.  For
     * example, the presence of crystal data is in the header so this
     * can be detected before any frame is read, but the crystal data
     * itself is only read when a frame is read in.
     */
    virtual bool hasPeriodicBox(void) const =0;
    //! Returns the periodic box for the current frame/trajectory
    virtual GCoord periodicBox(void) const =0;

    //! Returns the current frames coordinates as a vector of GCoords
    /** Some formats, notably DCDs, do not interleave their
     * coordinates.  This means that this could be a potentially
     * expensive operation.
     */
    virtual std::vector<GCoord> coords(void) =0;
  
    //! Update the coordinates in an AtomicGroup with the current frame.
    /** As with the coords() member function, some formats may have
     * non-interleaved coordinate data making the copying of coordinates
     * that much more expensive.
     *
     * In the case that the group is smaller than the trajectory frame,
     * it is assumed that the atomic-id's of the group are indices into
     * the frame's coordinates...  Since atom-id's usually begin with 1
     * and not 0, the indices are necessarily shifted by -1.
     *
     * If the trajectory has periodic boundary information, then the
     * group's periodicBox will also be updated.
     */
    virtual void updateGroupCoords(AtomicGroup& g) =0;

    //! Seek to the next frame in the sequence (used by readFrame() when
    //! operating as an iterator).
    void seekNextFrame(void);

    //! Seek to a specific frame, be it in the same contiguous file or
    //! in separate files.
    void seekFrame(const uint i);


    //! Parse an actual frame.
    /** parseFrame() is expected to read in a frame through the
     * Trajectory's StreamWrapper.  It returns a bool indicating whether
     * or not it was able to actually read a frame (i.e. false indicates
     * EOF).
     */
    virtual bool parseFrame(void) =0;

    //! Reads the next frame in a trajectory, returning false if at the end.
    bool readFrame(void);
      
    //! Reads a specific frame in a trajectory.
    /** Reading a specific frame also resets the readFrame() iterator
     * version so it will continue where readFrame(i) left off...
     */
    bool readFrame(const int i);

  protected:
    StreamWrapper ifs;
    bool cached_first;    // Indicates that the first frame is cached by
                          // the subclass...

  private:
    
    //! NVI implementation for seeking next frame
    virtual void seekNextFrameImpl() =0;

    //! NVI implementation for seeking a specific frame
    virtual void seekFrameImpl(const uint) =0;

    //! NVI implementation of rewind
    virtual void rewindImpl(void) =0;

  };

}
