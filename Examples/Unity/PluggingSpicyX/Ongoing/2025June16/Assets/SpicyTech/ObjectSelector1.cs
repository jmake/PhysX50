using UnityEngine;


public class ObjectSelector1 : MonoBehaviour
{
    private Material originalMaterial;
    private GameObject currentlySelected;

    [SerializeField] private Color highlightColor = Color.red;

    void Update()
    {
        if (IsLeftMouseClick())
        {
            TrySelectObjectUnderMouse();
        }

        if (IsLeftMouseHeld() && currentlySelected != null)
        {
            MoveSelectedObjectOnXZPlane();
        }

        if (IsLeftMouseReleased())
        {
            ResetCurrentlySelected();
        }
    }

    bool IsLeftMouseClick()
    {
        return Input.GetMouseButtonDown(0);
    }

    bool IsLeftMouseHeld()
    {
        return Input.GetMouseButton(0);
    }

    bool IsLeftMouseReleased()
    {
        return Input.GetMouseButtonUp(0);
    }


    Ray GetMouseRay()
    {
        return Camera.main.ScreenPointToRay(Input.mousePosition);
    }

    void HighlightObject(GameObject obj)
    {
        Renderer renderer = obj.GetComponent<Renderer>();
        if (renderer != null)
        {
            originalMaterial = renderer.material;
            Material highlightMat = new Material(originalMaterial);
            highlightMat.color = highlightColor;
            renderer.material = highlightMat;
        }
    }

    void ResetCurrentlySelected()
    {
        if (currentlySelected != null)
        {
            Renderer renderer = currentlySelected.GetComponent<Renderer>();
            if (renderer != null && originalMaterial != null)
            {
                renderer.material = originalMaterial;
            }

            currentlySelected = null;
            originalMaterial = null;
        }
    }


    void MoveSelectedObjectOnXZPlane()
    {
        Plane xzPlane = new Plane(Vector3.up, currentlySelected.transform.position);
        Ray ray = GetMouseRay();

        if (xzPlane.Raycast(ray, out float distance))
        {
            Vector3 point = ray.GetPoint(distance);
            Vector3 newPosition = new Vector3(point.x, currentlySelected.transform.position.y, point.z);
            //currentlySelected.transform.position = newPosition;
            parent.transform.position = newPosition;  
        }
    }


    void TrySelectObjectUnderMouse()
    {
        Ray ray = GetMouseRay();
        if (Physics.Raycast(ray, out RaycastHit hit))
        {
            GameObject targetObject = hit.collider.gameObject;

            if (targetObject == target)
            {
                HighlightObject(targetObject);
                currentlySelected = targetObject;
            }

        }
    }


    public void TargetSet(GameObject obj)
    {   
        target = obj; 
    }


    public void ParentSet(GameObject obj)
    {   
        parent = obj; 
    }

    GameObject target; 
    GameObject parent; 
}