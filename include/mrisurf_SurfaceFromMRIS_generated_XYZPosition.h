    namespace XYZPosition {
    struct Face : public Repr_Elt {
        inline Face                        (                                            );
        inline Face (                        Face const & src                           );
        inline Face (                        Representation* representation, size_t idx );
        inline Face (                        XYZPositionConsequencesM::Face const & src );
        inline Face (                        XYZPositionConsequences::Face const & src  );
        inline Face (                        DistortM::Face const & src                 );
        inline Face (                        Distort::Face const & src                  );
        inline Face (                        AnalysisM::Face const & src                );
        inline Face (                        Analysis::Face const & src                 );
        inline Face (                        AllM::Face const & src                     );
        int fno     () const { return idx; }

        inline Vertex v        ( size_t i  ) const ;
        inline char   ripflag  (           ) const ;
        inline char   oripflag (           ) const ;
        inline int    marked   (           ) const ;
        
        inline void set_ripflag  (  char to ) ;
        inline void set_oripflag (  char to ) ;
        inline void set_marked   (   int to ) ;
    };

    struct Vertex : public Repr_Elt {
        inline Vertex (                                                                     );
        inline Vertex (                        Vertex const & src                           );
        inline Vertex (                        Representation* representation, size_t idx   );
        inline Vertex (                        XYZPositionConsequencesM::Vertex const & src );
        inline Vertex (                        XYZPositionConsequences::Vertex const & src  );
        inline Vertex (                        DistortM::Vertex const & src                 );
        inline Vertex (                        Distort::Vertex const & src                  );
        inline Vertex (                        AnalysisM::Vertex const & src                );
        inline Vertex (                        Analysis::Vertex const & src                 );
        inline Vertex (                        AllM::Vertex const & src                     );
        int vno       () const { return idx; }

        // put the pointers before the ints, before the shorts, before uchars, to reduce size
        // the whole fits in much less than one cache line, so further ordering is no use
        inline Face   f                  ( size_t i  ) const ;  // size() is num.    array[v->num] the fno's of the neighboring faces         
        inline size_t n                  ( size_t i  ) const ;  // size() is num.    array[v->num] the face.v[*] index for this vertex        
        inline int    e                  ( size_t i  ) const ;  //  edge state for neighboring vertices                      
        inline Vertex v                  ( size_t i  ) const ;  // size() is vtotal.    array[v->vtotal or more] of vno, head sorted by hops     
        inline short  vnum               (           ) const ;  //  number of 1-hop neighbors    should use [p]VERTEXvnum(i, 
        inline short  v2num              (           ) const ;  //  number of 1, or 2-hop neighbors                          
        inline short  v3num              (           ) const ;  //  number of 1,2,or 3-hop neighbors                         
        inline short  vtotal             (           ) const ;  //  total # of neighbors. copy of vnum.nsizeCur              
        inline short  nsizeMaxClock      (           ) const ;  //  copy of mris->nsizeMaxClock when v#num                   
        inline uchar  nsizeMax           (           ) const ;  //  the max nsize that was used to fill in vnum etc          
        inline uchar  nsizeCur           (           ) const ;  //  index of the current v#num in vtotal                     
        inline uchar  num                (           ) const ;  //  number of neighboring faces                              
        // managed by MRISfreeDists[_orig] and MRISmakeDists[_orig]
        inline float  dist               ( size_t i  ) const ;  // size() is vtotal.    distance to neighboring vertices based on  xyz   
        inline float  dist_orig          ( size_t i  ) const ;  // size() is vtotal.    distance to neighboring vertices based on origxyz
        inline int    dist_capacity      (           ) const ;  //  -- should contain at least vtx_vtotal elements   
        inline int    dist_orig_capacity (           ) const ;  //  -- should contain at least vtx_vtotal elements   
        // 
        inline float  x                  (           ) const ;  //  current coordinates	
        inline float  y                  (           ) const ;  //  use MRISsetXYZ() to set
        inline float  z                  (           ) const ;
        // 
        inline float  origx              (           ) const ;  //  original coordinates, see also MRIS::origxyz_status
        inline float  origy              (           ) const ;  //  use MRISsetOriginalXYZ(, 
        inline float  origz              (           ) const ;  //  or MRISsetOriginalXYZfromXYZ to set
        inline float  cx                 (           ) const ;
        inline float  cy                 (           ) const ;
        inline float  cz                 (           ) const ;  //  coordinates in canonical coordinate system
        inline char   ripflag            (           ) const ;  //  vertex no longer exists - placed last to load the next vertex into cache
        inline void   which_coords       (int which, float *x, float *y, float *z) const ;
        // put the pointers before the ints, before the shorts, before uchars, to reduce size
        // the whole fits in much less than one cache line, so further ordering is no use
        
        inline void set_f       ( size_t i,   Face to ) ;  // size() is num.    array[v->num] the fno's of the neighboring faces         
        inline void set_n       ( size_t i, size_t to ) ;  // size() is num.    array[v->num] the face.v[*] index for this vertex        
        inline void set_e       ( size_t i,    int to ) ;  //  edge state for neighboring vertices                      
        inline void set_cx      (            float to ) ;
        inline void set_cy      (            float to ) ;
        inline void set_cz      (            float to ) ;  //  coordinates in canonical coordinate system
        inline void set_ripflag (             char to ) ;  //  vertex no longer exists - placed last to load the next vertex into cache
    };

    struct Surface : public Repr_Elt {
        inline Surface (                                               );
        inline Surface ( Surface const & src                           );
        inline Surface ( Representation* representation                );
        inline Surface ( XYZPositionConsequencesM::Surface const & src );
        inline Surface ( XYZPositionConsequences::Surface const & src  );
        inline Surface ( DistortM::Surface const & src                 );
        inline Surface ( Distort::Surface const & src                  );
        inline Surface ( AnalysisM::Surface const & src                );
        inline Surface ( Analysis::Surface const & src                 );
        inline Surface ( AllM::Surface const & src                     );

        // Fields being maintained by specialist functions
        inline int                   nverticesFrozen          (           ) const ;  //  # of vertices on surface is frozen
        inline int                   nvertices                (           ) const ;  //  # of vertices on surface, change by calling MRISreallocVerticesAndFaces et al
        inline int                   nfaces                   (           ) const ;  //  # of faces on surface, change by calling MRISreallocVerticesAndFaces et al
        inline bool                  faceAttachmentDeferred   (           ) const ;  //  defer connecting faces to vertices for performance reasons
        inline int                   nedges                   (           ) const ;  //  # of edges on surface
        inline int                   nstrips                  (           ) const ;
        inline Vertex                vertices                 ( size_t i  ) const ;
        inline p_p_void              dist_storage             (           ) const ;  //  the malloced/realloced vertex dist fields, so those fields can be quickly nulled and restored
        inline p_p_void              dist_orig_storage        (           ) const ;  //  the malloced/realloced vertex dist_orig fields, so those fields can be quickly nulled and restored
        inline int                   tempsAssigned            (           ) const ;  //  State of various temp fields that can be borrowed if not already in use
        inline Face                  faces                    ( size_t i  ) const ;
        inline MRI_EDGE              edges                    ( size_t i  ) const ;
        inline FaceNormCacheEntry    faceNormCacheEntries     ( size_t i  ) const ;
        inline FaceNormDeferredEntry faceNormDeferredEntries  ( size_t i  ) const ;
        inline int                   initialized              (           ) const ;
        inline PLTA                  lta                      (           ) const ;
        inline PMATRIX               SRASToTalSRAS_           (           ) const ;
        inline PMATRIX               TalSRASToSRAS_           (           ) const ;
        inline int                   free_transform           (           ) const ;
        inline double                radius                   (           ) const ;  //  radius (if status==MRIS_SPHERE)
        inline float                 a                        (           ) const ;
        inline float                 b                        (           ) const ;
        inline float                 c                        (           ) const ;  //  ellipsoid parameters
        inline MRIS_fname_t          fname                    (           ) const ;  //  file it was originally loaded from
        inline MRIS_Status           status                   (           ) const ;  //  type of surface (e.g. sphere, plane)
        inline MRIS_Status           origxyz_status           (           ) const ;  //  type of surface (e.g. sphere, plane) that this origxyz were obtained from
        inline int                   patch                    (           ) const ;  //  if a patch of the surface
        inline p_void                vp                       (           ) const ;  //  for misc. use
        inline float                 alpha                    (           ) const ;  //  rotation around z-axis
        inline float                 beta                     (           ) const ;  //  rotation around y-axis
        inline float                 gamma                    (           ) const ;  //  rotation around x-axis
        inline float                 da                       (           ) const ;
        inline float                 db                       (           ) const ;
        inline float                 dg                       (           ) const ;  //  old deltas
        inline int                   type                     (           ) const ;  //  what type of surface was this initially
        inline int                   max_vertices             (           ) const ;  //  may be bigger than nvertices, set by calling MRISreallocVerticesAndFaces
        inline int                   max_faces                (           ) const ;  //  may be bigger than nfaces,    set by calling MRISreallocVerticesAndFaces
        inline MRIS_subject_name_t   subject_name             (           ) const ;  //  name of the subject
        inline float                 canon_area               (           ) const ;
        inline int                   noscale                  (           ) const ;  //  don't scale by surface area if true
        inline float                 dx2                      ( size_t i  ) const ;  //  an extra set of gradient (not always alloced)
        inline float                 dy2                      ( size_t i  ) const ;
        inline float                 dz2                      ( size_t i  ) const ;
        inline PCOLOR_TABLE          ct                       (           ) const ;
        inline int                   useRealRAS               (           ) const ;  //  if 0 (default), vertex position is a conformed volume RAS with c_(r,"a","s")=0.  else is a real RAS (volume stored RAS)
        inline VOL_GEOM              vg                       (           ) const ;  //  volume info from which this surface is created. valid iff vg.valid = 1
        inline MRIS_cmdlines_t       cmdlines                 (           ) const ;
        inline int                   ncmds                    (           ) const ;
        inline float                 group_avg_surface_area   (           ) const ;  //  average of total surface area for group
        inline int                   group_avg_vtxarea_loaded (           ) const ;  //  average vertex area for group at each vertex
        inline int                   triangle_links_removed   (           ) const ;  //  for quad surfaces
        inline p_void                user_parms               (           ) const ;  //  for whatever the user wants to hang here
        inline PMATRIX               m_sras2vox               (           ) const ;  //  for converting surface ras to voxel
        inline PMRI                  mri_sras2vox             (           ) const ;  //  volume that the above matrix is for
        inline p_void                mht                      (           ) const ;
        inline p_void                temps                    (           ) const ;
        
        inline void set_vp    (  p_void to ) ;  //  for misc. use
        inline void set_alpha (   float to ) ;  //  rotation around z-axis
        inline void set_beta  (   float to ) ;  //  rotation around y-axis
        inline void set_gamma (   float to ) ;  //  rotation around x-axis
        inline void set_da    (   float to ) ;
        inline void set_db    (   float to ) ;
        inline void set_dg    (   float to ) ;  //  old deltas
        inline void set_type  (     int to ) ;  //  what type of surface was this initially
    };

    } // namespace XYZPosition
